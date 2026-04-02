@echo off
setlocal enabledelayedexpansion

:: 清理并创建输出目录
del /f /s /q outflie
mkdir outflie > nul

windres -i "demo.rc" -o "./outflie/demo.o"

:: MinGW 版本
set CC=gcc
set CFLAGS=-O0 -Wall
set OUTPUT=comx.exe

:: 初始化对象文件列表
set OBJS=

:: 编译 C_MyLib 中的源文件
for %%f in (./C_MyLib/*.c) do (
    echo 正在编译 %%f...
    %CC% %CFLAGS% -c "./C_MyLib/%%f" -o "./outflie/%%~nf.o"
    if errorlevel 1 (
        echo 编译失败: ./C_MyLib/%%f
        exit /b 1
    )
    set OBJS=!OBJS! "./outflie/%%~nf.o"
)

:: 编译主程序
echo 正在编译 comx.c...
%CC% %CFLAGS% -c comx.c -o "./outflie/comx.o"
if errorlevel 1 (
    echo 编译失败: comx.c
    exit /b 1
)
set OBJS=!OBJS! "./outflie/comx.o"

:: 显示将要链接的对象文件
echo 链接对象文件: !OBJS!

:: 链接所有目标文件
echo 正在链接 %OUTPUT%...
%CC% %CFLAGS% !OBJS! "./outflie/demo.o" -o %OUTPUT% -lsetupapi -luuid
if errorlevel 1 (
    echo 链接失败
    exit /b 1
)

echo 编译成功: %OUTPUT%
endlocal

timeout 2