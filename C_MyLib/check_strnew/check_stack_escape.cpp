// =================================================================
// GCC 栈逃逸追踪检测插件 - 前向污点分析（工业级防误报完全体）
// =================================================================
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

// 剥离并获取基础变量节点
tree get_base_var(tree node) {
    if (!node)
        return NULL_TREE;
    while (TREE_CODE(node) == COMPONENT_REF || TREE_CODE(node) == ARRAY_REF ||
           TREE_CODE(node) == MEM_REF || TREE_CODE(node) == VIEW_CONVERT_EXPR ||
           CONVERT_EXPR_P(node)) {
        node = TREE_OPERAND(node, 0);
    }
    if (TREE_CODE(node) == SSA_NAME) {
        return SSA_NAME_VAR(node);
    }
    return node;
}

// 核心判定机制：评估一个表达式是否真正携带了本地栈的“内存地址”
bool expr_is_stack_address(tree expr, const std::set<tree>& stack_vars, const std::set<tree>& tainted_vars) {
    if (!expr)
        return false;

    // 【修复点】使用 INTEGRAL_TYPE_P 覆盖 int/char，使用 SCALAR_FLOAT_TYPE_P 覆盖 float/double
    tree type = TREE_TYPE(expr);
    if (type && (INTEGRAL_TYPE_P(type) || SCALAR_FLOAT_TYPE_P(type)) && !POINTER_TYPE_P(type)) {
        return false;
    }

    // 剥离外层包裹
    while (CONVERT_EXPR_P(expr) || TREE_CODE(expr) == VIEW_CONVERT_EXPR || TREE_CODE(expr) == NOP_EXPR) {
        expr = TREE_OPERAND(expr, 0);
    }

    // 场景 A: 显式取地址操作（如 &local_buf）
    if (TREE_CODE(expr) == ADDR_EXPR) {
        tree op = TREE_OPERAND(expr, 0);
        // 如果是常量字符串（.rodata区），绝对安全，直接放行
        if (TREE_CODE(op) == STRING_CST)
            return false;

        tree base = get_base_var(op);
        if (base && stack_vars.count(base))
            return true;
    }

    // 场景 B: 变量本身就是一个被污染的指针、或者本地数组名本身
    tree base = get_base_var(expr);
    if (base) {
        if (stack_vars.count(base) && TREE_CODE(TREE_TYPE(base)) == ARRAY_TYPE) {
            return true;
        }
        if (tainted_vars.count(base)) {
            return true;
        }
    }

    // 场景 C: 递归检索子表达式
    for (unsigned i = 0; i < TREE_OPERAND_LENGTH(expr); i++) {
        if (expr_is_stack_address(TREE_OPERAND(expr, i), stack_vars, tainted_vars))
            return true;
    }

    return false;
}

// 判定当前写入的目的地（左值）是否属于“外部逃逸通道”
bool is_escape_sink_lhs(tree lhs) {
    if (!lhs)
        return false;

    tree base = lhs;
    while (TREE_CODE(base) == COMPONENT_REF || TREE_CODE(base) == ARRAY_REF ||
           TREE_CODE(base) == VIEW_CONVERT_EXPR || CONVERT_EXPR_P(base)) {
        base = TREE_OPERAND(base, 0);
    }

    // 1. 全局变量或静态变量
    if (TREE_CODE(base) == VAR_DECL && (TREE_STATIC(base) || DECL_EXTERNAL(base) || TREE_PUBLIC(base))) {
        return true;
    }

    // 2. 指针解引用写入（如 *ptr = ...）
    if (TREE_CODE(base) == MEM_REF) {
        tree ptr = TREE_OPERAND(base, 0);
        while (TREE_CODE(ptr) == SSA_NAME || CONVERT_EXPR_P(ptr) || TREE_CODE(ptr) == VIEW_CONVERT_EXPR) {
            if (TREE_CODE(ptr) == SSA_NAME) {
                tree var = SSA_NAME_VAR(ptr);
                if (var && (TREE_CODE(var) == PARM_DECL || TREE_STATIC(var) || DECL_EXTERNAL(var) || TREE_PUBLIC(var)))
                    return true;
                break;
            }
            ptr = TREE_OPERAND(ptr, 0);
        }
        if (TREE_CODE(ptr) == PARM_DECL)
            return true;
        if (TREE_CODE(ptr) == VAR_DECL && (TREE_STATIC(ptr) || DECL_EXTERNAL(ptr) || TREE_PUBLIC(ptr)))
            return true;
    }

    return false;
}

// 智能标准库参数过滤器
bool call_argument_can_leak(const char *fname, unsigned arg_idx) {
    if (!fname)
        return true;
    if (strcmp(fname, "strstr") == 0)
        return (arg_idx == 0);
    if (strcmp(fname, "memcpy") == 0 || strcmp(fname, "memmove") == 0 ||
        strcmp(fname, "strcpy") == 0 || strcmp(fname, "strncpy") == 0 ||
        strcmp(fname, "strcat") == 0 || strcmp(fname, "strncat") == 0) {
        return (arg_idx == 0);
    }
    if (strcmp(fname, "strlen") == 0 || strcmp(fname, "strcmp") == 0 ||
        strcmp(fname, "strncmp") == 0 || strcmp(fname, "strcasecmp") == 0 ||
        strcmp(fname, "strncasecmp") == 0 || strcmp(fname, "memcmp") == 0 ||
        strcmp(fname, "atoi") == 0 || strcmp(fname, "atol") == 0 ||
        strcmp(fname, "strtol") == 0 || strcmp(fname, "strtoul") == 0 ||
        strcmp(fname, "printf") == 0 || strcmp(fname, "sprintf") == 0 ||
        strcmp(fname, "snprintf") == 0 || strcmp(fname, "fprintf") == 0) {
        return false;
    }
    return true;
}

const pass_data exit_defense_pass_data = {
    GIMPLE_PASS, "exit_defense_check", OPTGROUP_NONE, TV_NONE,
    PROP_ssa, 0, 0, 0, 0};

struct exit_defense_pass : public gimple_opt_pass {
    exit_defense_pass(gcc::context *ctxt)
        : gimple_opt_pass(exit_defense_pass_data, ctxt) {
    }

    virtual unsigned int execute(function *fun) override {
        std::set<tree> stack_vars;   // 仅存放真正的本地物理栈变量（作为判定目标）
        std::set<tree> tainted_vars; // 存放接收了栈地址、处于扩散阶段的中间指针/结构体

        // 步骤 1：收集当前函数所有局部栈变量
        tree var;
        unsigned int i;
        FOR_EACH_LOCAL_DECL(fun, i, var) {
            if (TREE_CODE(var) == VAR_DECL && !TREE_STATIC(var) && !DECL_EXTERNAL(var)) {
                stack_vars.insert(var);
            }
        }
        if (stack_vars.empty())
            return 0;

        // 步骤 2：不动点迭代扩散污点（仅追踪地址的流向）
        bool changed = true;
        while (changed) {
            changed = false;
            basic_block bb;
            FOR_EACH_BB_FN(bb, fun) {
                for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                    gimple *stmt = gsi_stmt(gsi);

                    if (is_gimple_assign(stmt)) {
                        tree lhs = gimple_assign_lhs(stmt);
                        if (!lhs)
                            continue;

                        bool rhs_has_stack = false;
                        for (unsigned j = 1; j < gimple_num_ops(stmt); j++) {
                            if (expr_is_stack_address(gimple_op(stmt, j), stack_vars, tainted_vars)) {
                                rhs_has_stack = true;
                                break;
                            }
                        }

                        if (rhs_has_stack && !is_escape_sink_lhs(lhs)) {
                            tree lhs_base = get_base_var(lhs);
                            if (lhs_base && !tainted_vars.count(lhs_base)) {
                                tainted_vars.insert(lhs_base);
                                changed = true;
                            }
                        }
                    } else if (gimple_code(stmt) == GIMPLE_CALL) {
                        gcall *call_stmt = as_a<gcall *>(stmt);
                        tree lhs = gimple_call_lhs(call_stmt);
                        if (!lhs)
                            continue;

                        tree fndecl = gimple_call_fndecl(call_stmt);
                        const char *fname = (fndecl && DECL_NAME(fndecl)) ? IDENTIFIER_POINTER(DECL_NAME(fndecl)) : NULL;

                        bool args_has_stack = false;
                        for (unsigned j = 0; j < gimple_call_num_args(call_stmt); j++) {
                            if (call_argument_can_leak(fname, j) && expr_is_stack_address(gimple_call_arg(call_stmt, j), stack_vars, tainted_vars)) {
                                args_has_stack = true;
                                break;
                            }
                        }

                        if (args_has_stack && !is_escape_sink_lhs(lhs)) {
                            tree lhs_base = get_base_var(lhs);
                            if (lhs_base && !tainted_vars.count(lhs_base)) {
                                tainted_vars.insert(lhs_base);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }

        // 步骤 3：精确安全审计与收网拦截
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);

                // 拦截 1: 显式 Return 逃逸
                if (gimple_code(stmt) == GIMPLE_RETURN) {
                    greturn *ret_stmt = as_a<greturn *>(stmt);
                    tree retval = gimple_return_retval(ret_stmt);
                    if (retval && expr_is_stack_address(retval, stack_vars, tainted_vars)) {
                        warning_at(gimple_location(stmt), 0,
                                   "Security Alert: Function capturing local stack address returns a value that leaks from the scope.");
                    }
                }
                // 拦截 2: 赋值给外部终点 (全局变量或指针传出参数)
                else if (is_gimple_assign(stmt)) {
                    tree lhs = gimple_assign_lhs(stmt);
                    if (!lhs)
                        continue;

                    bool rhs_has_stack = false;
                    for (unsigned j = 1; j < gimple_num_ops(stmt); j++) {
                        if (expr_is_stack_address(gimple_op(stmt, j), stack_vars, tainted_vars)) {
                            rhs_has_stack = true;
                            break;
                        }
                    }

                    if (rhs_has_stack && is_escape_sink_lhs(lhs)) {
                        warning_at(gimple_location(stmt), 0,
                                   "Security Alert: Local stack memory flows into an escaping destination (global variable or out-parameter).");
                    }
                }
                // 拦截 3: 函数调用直接将堆栈地址送入外部逃逸通道
                else if (gimple_code(stmt) == GIMPLE_CALL) {
                    gcall *call_stmt = as_a<gcall *>(stmt);
                    tree lhs = gimple_call_lhs(call_stmt);

                    tree fndecl = gimple_call_fndecl(call_stmt);
                    const char *fname = (fndecl && DECL_NAME(fndecl)) ? IDENTIFIER_POINTER(DECL_NAME(fndecl)) : NULL;

                    bool args_has_stack = false;
                    for (unsigned j = 0; j < gimple_call_num_args(call_stmt); j++) {
                        if (call_argument_can_leak(fname, j) && expr_is_stack_address(gimple_call_arg(call_stmt, j), stack_vars, tainted_vars)) {
                            args_has_stack = true;
                            break;
                        }
                    }

                    if (args_has_stack && lhs && is_escape_sink_lhs(lhs)) {
                        warning_at(gimple_location(stmt), 0,
                                   "Security Alert: Function call passes local stack memory to an escaping destination.");
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
