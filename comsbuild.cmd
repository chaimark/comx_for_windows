@echo off
setlocal enabledelayedexpansion

:: 清理并创建输出目录
del /f /s /q outflie
mkdir outflie > nul

windres -i "demo.rc" -o "./outflie/demo.o"

:: MinGW 版本
set CC=gcc
set CFLAGS=-O0 -Wall
set OUTPUT=coms.exe

:: 初始化对象文件列表
set OBJS=

:: 编译 WorkLib 中的源文件
for %%f in (./WorkLib/*.c) do (
    echo 正在编译 %%f...  
    powershell -Command "& { ./WorkLib/windows_check_strnew.ps1 ./WorkLib/%%f }
	%CC% %CFLAGS% -c "./WorkLib/%%f" -o "./outflie/%%~nf.o"
    if errorlevel 1 (
        echo 编译失败: ./WorkLib/%%f
        exit /b 1
    )
    set OBJS=!OBJS! "./outflie/%%~nf.o"
)

:: 编译主程序
echo 正在编译 coms.c...
powershell -Command "& { ./WorkLib/windows_check_strnew.ps1 ./coms.c }
%CC% %CFLAGS% -c coms.c -o "./outflie/coms.o"
if errorlevel 1 (
    echo 编译失败: coms.c
    exit /b 1
)
set OBJS=!OBJS! "./outflie/coms.o"

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
