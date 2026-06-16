#!/bin/bash

# 获取传入的文件路径
FILE_PATH=$1

# 检查参数是否为空
if [ -z "$FILE_PATH" ]; then
    echo -e "\e[31m[ERROR] Use Rules: ./check_strnew.sh ./test_strnew.c\e[0m"
    exit 1
fi

# 检查文件是否存在
if [ ! -f "$FILE_PATH" ]; then
    echo -e "\e[31m[ERROR] find file Fail: $FILE_PATH\e[0m"
    exit 1
fi

# 使用 perl 清除 C 语言的单行 (//) 和多行 (/* */) 注释
CLEANED_CONTENT=$(perl -0777 -pe 's/\/\/.*//g; s/\/\*[\s\S]*?\*\///g' "$FILE_PATH")

# 初始化状态变量
CURRENT_FUNCTION="Global"
TOTAL_ERRORS=0
LINE_NUM=0

# 声明关联数组（模拟 PowerShell 的 HashSet）
declare -A LOCAL_ARRAYS
declare -A TAINTED_VARS

# ==================== 预定义正则表达式变量 (解决 Linux 语法报错的核心) ====================
REGEX_FUNC='(^|[^a-zA-Z0-9_])(strnew|void|int|char)\*?[[:space:]]+([a-zA-Z0-9_]+)[[:space:]]*\([^)]*\)[[:space:]]*\{'
REGEX_ARRAY='(^|[^a-zA-Z0-9_])(char|int[0-9]*_t|uint[0-9]*_t|int)[[:space:]]+([a-zA-Z0-9_]+)[[:space:]]*\[[^]]+\]'
REGEX_NEWSTR='(^|[^a-zA-Z0-9_])newString[[:space:]]*\([[:space:]]*([a-zA-Z0-9_]+)'
REGEX_NEWNAME='([a-zA-Z0-9_]+)[[:space:]]*=[[:space:]]*NEW_NAME[[:space:]]*\([[:space:]]*([a-zA-Z0-9_]+)[[:space:]]*\)'
REGEX_ALIAS='([a-zA-Z0-9_]+)[[:space:]]*=[[:space:]]*&?([a-zA-Z0-9_]+)([^a-zA-Z0-9_]|$)'
REGEX_GLOBAL='(^|[^a-zA-Z0-9_])(g_[a-zA-Z0-9_]+|g_saved_str)[[:space:]]*=[[:space:]]*&?([a-zA-Z0-9_]+)([^a-zA-Z0-9_]|$)'
REGEX_RETURN='(^|[^a-zA-Z0-9_])return[[:space:]]+([a-zA-Z0-9_]+)([^a-zA-Z0-9_]|$)'
# =======================================================================================

# 按行读取清除注释后的文本内容
while IFS= read -r line || [ -n "$line" ]; do
    LINE_NUM=$((LINE_NUM + 1))

    # 去除两端的空格
    line="${line#${line%%[![:space:]]*}}"
    line="${line%${line##*[![:space:]]}}"

    # 空行直接跳过
    [ -z "$line" ] && continue

    # 1. 匹配函数头进入新函数
    if [[ "$line" =~ $REGEX_FUNC ]]; then
        CURRENT_FUNCTION="${BASH_REMATCH[3]}"
        # 清空关联数组
        unset LOCAL_ARRAYS && declare -A LOCAL_ARRAYS
        unset TAINTED_VARS && declare -A TAINTED_VARS
        continue
    fi

    # 2. 匹配函数结束
    if [ "$line" = "}" ]; then
        CURRENT_FUNCTION="Global"
        continue
    fi

    # 如果在全局区域，不进行逃逸分析
    if [ "$CURRENT_FUNCTION" = "Global" ]; then
        continue
    fi

    # 3. 识别本地栈空间数组
    if [[ "$line" =~ $REGEX_ARRAY ]]; then
        var_name="${BASH_REMATCH[3]}"
        LOCAL_ARRAYS["$var_name"]=1
    fi

    # 4. 识别 newString 宏
    if [[ "$line" =~ $REGEX_NEWSTR ]]; then
        var_name="${BASH_REMATCH[2]}"
        TAINTED_VARS["$var_name"]=1
    fi

    # 5. NEW_NAME 绑定本地栈数组
    if [[ "$line" =~ $REGEX_NEWNAME ]]; then
        var="${BASH_REMATCH[1]}"
        src="${BASH_REMATCH[2]}"
        if [ "${LOCAL_ARRAYS[$src]}" = "1" ]; then
            TAINTED_VARS["$var"]=1
        fi
    fi

    # 6. 指针别名传递
    if [[ "$line" =~ $REGEX_ALIAS ]]; then
        dest="${BASH_REMATCH[1]}"
        src="${BASH_REMATCH[2]}"
        if [ "${TAINTED_VARS[$src]}" = "1" ]; then
            TAINTED_VARS["$dest"]=1
        fi
    fi

    # 7. 检查逃逸点 A: 赋值给全局变量 (修改为 GCC warning 格式)
    if [[ "$line" =~ $REGEX_GLOBAL ]]; then
        glob="${BASH_REMATCH[2]}"
        src="${BASH_REMATCH[3]}"
        if [ "${TAINTED_VARS[$src]}" = "1" ]; then
            echo -e "$FILE_PATH:$LINE_NUM: \e[35mwarning:\e[0m [function $CURRENT_FUNCTION] Variable '$src' has been assigned to the global variable '$glob'"
            TOTAL_ERRORS=$((TOTAL_ERRORS + 1))
        fi
    fi

    # 8. 检查逃逸点 B: 危险的 return 返回栈数据 (修改为 GCC error 格式)
    if [[ "$line" =~ $REGEX_RETURN ]]; then
        ret_var="${BASH_REMATCH[2]}"
        if [ "${TAINTED_VARS[$ret_var]}" = "1" ]; then
            echo -e "$FILE_PATH:$LINE_NUM: \e[31;1merror:\e[0m [function $CURRENT_FUNCTION] The variable '$ret_var' is stack data"
            TOTAL_ERRORS=$((TOTAL_ERRORS + 1))
        fi
    fi

done <<< "$CLEANED_CONTENT"

# 最终判定退出状态
if [ "$TOTAL_ERRORS" -eq 0 ]; then
    exit 0
else
    echo "=================================================================="
    echo -e "\e[31m[DONE] check over: find $TOTAL_ERRORS errors\e[0m"
    exit 1
fi
