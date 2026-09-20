@echo off
setlocal enabledelayedexpansion

:: 清理并重建输出目录
if exist outflie rmdir /s /q outflie
mkdir outflie > nul

windres -i "demo.rc" -o "./outflie/demo.o"
if errorlevel 1 (
    echo windres 失败
    exit /b 1
)

:: MinGW 编译器设置
set CC=gcc
set CFLAGS=-O0 -Wall

:: 初始化对象文件列表
set OBJS=

:: 编译 WorkLib 中的所有源文件
for %%f in (./WorkLib/*.c) do (
    echo 正在编译 %%f...
    powershell -Command "& { ./WorkLib/windows_check_strnew.ps1 ./WorkLib/%%f } "
    %CC% %CFLAGS% -c "./WorkLib/%%f" -o "./outflie/%%~nf.o"
    if errorlevel 1 (
        echo 编译失败: ./WorkLib/%%f
        exit /b 1
    )
    set OBJS=!OBJS! "./outflie/%%~nf.o"
)

:: 保存基础对象文件（WorkLib的对象文件）
set BASE_OBJS=!OBJS!

:: 需要编译的主程序列表
set MAIN_PROGS=comh coms comx

:: 循环编译每个主程序
for %%p in (%MAIN_PROGS%) do (
    echo ============================================
    echo 正在编译 %%p.c...
    powershell -Command "& { ./WorkLib/windows_check_strnew.ps1 ./%%p.c } "
    %CC% %CFLAGS% -c %%p.c -o "./outflie/%%p.o"
    if errorlevel 1 (
        echo 编译失败: %%p.c
        exit /b 1
    )

    :: 重置OBJS为基础对象文件+当前主程序对象文件
    set OBJS=%BASE_OBJS% "./outflie/%%p.o"

    :: 显示要链接的对象文件
    echo 链接对象文件: !OBJS!

    :: 设置输出文件名（使用延迟展开）
    set OUTPUT=%%p.exe

    :: 链接生成可执行文件（注意使用 !OUTPUT! 而不是 %OUTPUT%）
    echo 正在链接 !OUTPUT!...
    %CC% %CFLAGS% !OBJS! "./outflie/demo.o" -o !OUTPUT! -lsetupapi -luuid
    if errorlevel 1 (
        echo 链接失败: !OUTPUT!
        exit /b 1
    )
    echo 编译成功: !OUTPUT!
)

echo ============================================
echo 所有程序构建完成！

endlocal
timeout 2