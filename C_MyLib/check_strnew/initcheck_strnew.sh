#!/bin/bash

# Usage: ./build_plugin.sh [native|cross]
#   native: 使用 g++ 编译（默认）
#   cross:  使用 aarch64-linux-gnu-g++ 编译

build_mode=${1:-native}
sudo apt update

case "$build_mode" in
native)
    CXX=g++
    echo "使用 $CXX 编译插件"
    sudo apt install -y gcc-11-plugin-dev
    ;;
cross)
    exit 1
    # CXX=aarch64-linux-gnu-g++
    # echo "使用 $CXX 编译插件"
    ;;
*)
    echo "错误：未知模式 '$build_mode'。请使用 'native' 或 'cross'。"
    exit 1
    ;;
esac

# 获取当前编译器对应的 plugin 头文件路径
plugin_inc=$($CXX -print-file-name=plugin)/include
if [ ! -d "$plugin_inc" ]; then
    echo "错误：无法找到插件头文件目录 $plugin_inc"
    echo "请确保已安装对应编译器的插件开发包。"
    exit 1
fi

echo "使用插件头文件路径: $plugin_inc"

# 编译插件
$CXX -I"$plugin_inc" -fPIC -shared -fno-rtti -O2 check_stack_escape.cpp -o check_stack_escape.so

if [ $? -eq 0 ]; then
    echo "编译成功: check_stack_escape.so"
else
    echo "编译失败"
    exit 1
fi
