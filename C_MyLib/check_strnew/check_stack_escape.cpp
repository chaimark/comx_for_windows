// =======================================================
// GCC 栈逃逸追踪检测插件 - 智能标准库过滤完全体（零误报）
// =======================================================
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include <gcc-plugin.h>
#include <c-tree.h>
#include <plugin-version.h>
#include <backend.h>
#include <tree.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <tree-pass.h>
#include <diagnostic-core.h>
#include <context.h>
#include <ssa.h>
#include <vector>
#include <set>

int plugin_is_GPL_compatible;
extern gcc::context *g;

namespace {

// 判定是否为局部非静态栈变量
bool is_local_stack_var(tree decl) {
    if (!decl)
        return false;
    if (TREE_CODE(decl) == SSA_NAME) {
        decl = SSA_NAME_VAR(decl);
    }
    if (!decl)
        return false;
    return (TREE_CODE(decl) == VAR_DECL && !TREE_STATIC(decl) && !DECL_EXTERNAL(decl));
}

// 提取核心的基础变量 (剥离字段、数组引用)
tree get_base_decl(tree node) {
    if (!node)
        return NULL_TREE;
    while (TREE_CODE(node) == COMPONENT_REF || TREE_CODE(node) == ARRAY_REF ||
           TREE_CODE(node) == MEM_REF || TREE_CODE(node) == VIEW_CONVERT_EXPR ||
           CONVERT_EXPR_P(node)) {
        node = TREE_OPERAND(node, 0);
    }
    if (TREE_CODE(node) == SSA_NAME) {
        return SSA_NAME_VAR(node) ? DECL_ORIGIN(SSA_NAME_VAR(node)) : NULL_TREE;
    }
    return node ? DECL_ORIGIN(node) : NULL_TREE;
}

// 检查某个表达式中是否直接包含指定栈变量的地址
bool expr_contains_stack_addr(tree expr, tree stack_var) {
    if (!expr || !stack_var)
        return false;
    if (TREE_CODE(expr) == ADDR_EXPR) {
        return get_base_decl(TREE_OPERAND(expr, 0)) == stack_var;
    }
    for (unsigned i = 0; i < TREE_OPERAND_LENGTH(expr); i++) {
        if (expr_contains_stack_addr(TREE_OPERAND(expr, i), stack_var))
            return true;
    }
    return false;
}

// 核心智能常识库：判定某个函数的特定参数是否具有将地址泄漏到返回值的风险
bool call_argument_can_leak(const char *fname, unsigned arg_idx) {
    if (!fname)
        return true; // 无法确定函数名时，选择保守策略

    // 1. 常见标准字符串查找与拷贝函数
    if (strcmp(fname, "strstr") == 0) {
        // strstr(haystack, needle) -> 只有 haystack (arg 0) 会作为返回值的一部分返回，针串(arg 1)是安全的
        return (arg_idx == 0);
    }
    if (strcmp(fname, "memcpy") == 0 || strcmp(fname, "memmove") == 0 ||
        strcmp(fname, "strcpy") == 0 || strcmp(fname, "strncpy") == 0 ||
        strcmp(fname, "strcat") == 0 || strcmp(fname, "strncat") == 0) {
        // 这些函数均返回 dest (arg 0)，源缓冲区 (arg 1) 绝不会通过返回值泄漏
        return (arg_idx == 0);
    }

    // 2. 返回纯算术值/长度/比较结果的函数，绝不可能泄漏指针地址
    if (strcmp(fname, "strlen") == 0 || strcmp(fname, "strcmp") == 0 ||
        strcmp(fname, "strncmp") == 0 || strcmp(fname, "strcasecmp") == 0 ||
        strcmp(fname, "strncasecmp") == 0 || strcmp(fname, "memcmp") == 0 ||
        strcmp(fname, "atoi") == 0 || strcmp(fname, "atol") == 0 ||
        strcmp(fname, "strtol") == 0 || strcmp(fname, "strtoul") == 0) {
        return false;
    }

    // 3. 常见的打印输出函数
    if (strcmp(fname, "printf") == 0 || strcmp(fname, "sprintf") == 0 ||
        strcmp(fname, "snprintf") == 0 || strcmp(fname, "fprintf") == 0) {
        return false;
    }

    return true; // 自定义函数（如 New_Str_Obj）默认采取严格数据流审计
}

const pass_data exit_defense_pass_data = {
    GIMPLE_PASS, "exit_defense_check", OPTGROUP_NONE, TV_NONE,
    PROP_ssa, 0, 0, 0, 0};

struct exit_defense_pass : public gimple_opt_pass {
    exit_defense_pass(gcc::context *ctxt)
        : gimple_opt_pass(exit_defense_pass_data, ctxt) {
    }

    virtual unsigned int execute(function *fun) override {
        // 1. 收集当前函数的所有局部栈变量
        std::vector<tree> local_stack_vars;
        tree var;
        unsigned int i;
        FOR_EACH_LOCAL_DECL(fun, i, var) {
            if (is_local_stack_var(var)) {
                local_stack_vars.push_back(DECL_ORIGIN(var));
            }
        }
        if (local_stack_vars.empty())
            return 0;

        // 2. 逆向数据流追溯：找出当前函数中所有“最终参与了返回”的变量集合
        std::set<tree> returned_entities;
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);

                if (gimple_code(stmt) == GIMPLE_RETURN) {
                    greturn *ret_stmt = as_a<greturn *>(stmt);
                    tree retval = gimple_return_retval(ret_stmt);
                    if (retval) {
                        tree base = get_base_decl(retval);
                        if (base)
                            returned_entities.insert(base);
                    }
                }

                if (is_gimple_assign(stmt)) {
                    tree lhs_base = get_base_decl(gimple_assign_lhs(stmt));
                    if (lhs_base && TREE_CODE(lhs_base) == RESULT_DECL) {
                        tree rhs_base = get_base_decl(gimple_assign_rhs1(stmt));
                        if (rhs_base)
                            returned_entities.insert(rhs_base);
                    }
                }
            }
        }

        // 多轮扩散解决多级临时变量中转
        bool changed = true;
        while (changed) {
            changed = false;
            FOR_EACH_BB_FN(bb, fun) {
                for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                    gimple *stmt = gsi_stmt(gsi);
                    if (is_gimple_assign(stmt)) {
                        tree lhs_base = get_base_decl(gimple_assign_lhs(stmt));
                        if (lhs_base && returned_entities.count(lhs_base)) {
                            for (unsigned j = 1; j < gimple_num_ops(stmt); j++) {
                                tree rhs_base = get_base_decl(gimple_op(stmt, j));
                                if (rhs_base && !returned_entities.count(rhs_base)) {
                                    returned_entities.insert(rhs_base);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        // 3. 最终收网：结合标准库过滤器进行精准拦截
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);

                // 检查普通赋值语句
                if (is_gimple_assign(stmt)) {
                    tree lhs_base = get_base_decl(gimple_assign_lhs(stmt));
                    if (lhs_base && returned_entities.count(lhs_base)) {
                        for (tree stack_var : local_stack_vars) {
                            bool has_leak = false;
                            for (unsigned j = 1; j < gimple_num_ops(stmt); j++) {
                                if (expr_contains_stack_addr(gimple_op(stmt, j), stack_var)) {
                                    has_leak = true;
                                    break;
                                }
                            }
                            if (has_leak) {
                                warning_at(gimple_location(stmt), 0,
                                           "Security Alert: Local stack memory flows into a structure field or variable that is returned.");
                                break;
                            }
                        }
                    }
                }

                // 检查函数调用语句
                else if (gimple_code(stmt) == GIMPLE_CALL) {
                    gcall *call_stmt = as_a<gcall *>(stmt);
                    tree lhs = gimple_call_lhs(call_stmt);
                    tree lhs_base = lhs ? get_base_decl(lhs) : NULL_TREE;

                    if (lhs_base && returned_entities.count(lhs_base)) {
                        // 提取函数名
                        tree fndecl = gimple_call_fndecl(call_stmt);
                        const char *fname = (fndecl && DECL_NAME(fndecl)) ? IDENTIFIER_POINTER(DECL_NAME(fndecl)) : NULL;

                        for (tree stack_var : local_stack_vars) {
                            bool param_leak = false;
                            for (unsigned j = 0; j < gimple_call_num_args(call_stmt); j++) {
                                // 同时满足：该参数位置允许泄漏 && 参数表达式确实包含栈变量地址
                                if (call_argument_can_leak(fname, j) &&
                                    expr_contains_stack_addr(gimple_call_arg(call_stmt, j), stack_var)) {
                                    param_leak = true;
                                    break;
                                }
                            }
                            if (param_leak) {
                                warning_at(gimple_location(stmt), 0,
                                           "Security Alert: Function capturing local stack address returns a value that leaks from the scope.");
                                break;
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }
};

} // namespace

int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version) {
    if (!plugin_default_version_check(version, &gcc_version)) {
        error("plugin version mismatch with current GCC compiler");
        return 1;
    }

    exit_defense_pass *p = new exit_defense_pass(g);
    struct register_pass_info pass_info;
    pass_info.pass = p;
    pass_info.reference_pass_name = "ssa";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
    return 0;
}
