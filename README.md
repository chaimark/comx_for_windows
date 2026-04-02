## comx for windows
	这是一个在 Windows 终端环境下使用的串口终端

## 使用方法1：
	如果有必要，可以直接将该程序的路径加到环境变量，在 win+r 输入 comx 调用

## 使用方法2：
	在 wsl 中直接调用 exe
	
	可以编写类似的脚本，实现在 wsl 下调用
		#!/bin/bash
		clear

		cd D_promgrams/
		./comx.exe
	需要注意的是，D_promgrams 是程序在 wsl 下的软连接，需要自行创建

## build.cmd 
	build.cmd 是一键编译脚本，执行后生成的 comx.exe 就是该工具
