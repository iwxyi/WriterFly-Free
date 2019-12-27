@echo off
setlocal enabledelayedexpansion
rem 参数：代码目录 项目名（一般项目名就是代码目录）
set vdir=%1
set vpro=%2
set qmk="0"
set run=%3
set rdir=%vdir%_Win32

rem 设置调试目录
cd ..
if not exist %rdir% ( md %rdir% )
cd %rdir%

rem 判断需不需要qmake
set file_1="Makefile"
set file_2="%vdir%/%vpro%.pro"
call :mark1 %file_1% %file_2%

:compare
rem 编译源代码
mingw32-make.exe -j8

if not "%run%"=="" (
	rem 运行程序
	echo "run application"
	cd release && %vpro%.exe

	rem 回到原来的目录
	cd ../../%vpro%
)

echo "build finish"

:mark1
rem 参数：要判断的文件1 比 2 新
if !qmk! EQU "0" (
	if exist %~1 (
		if "%~t1" GEQ "%~t2" (
			set "qmk=2"
			echo "skip qmake"
		)
	)

	if !qmk! EQU "0" (
		set qmk="1"
		echo "exec qmake"
		rem 通过qmake生成Makefile
		D:\IDE\Qt5.13.1\5.13.1\mingw73_32\bin\qmake.exe %vdir%/%vpro%.pro -spec win32-g++ "CONFIG+=qtquickcompiler" && D:/IDE/Qt5.13.1/Tools/mingw730_32/bin/mingw32-make.exe qmake_all
		)
	)
)


