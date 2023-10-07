@echo off

rem NOTE: We need to change this in the build_installer.nsi as well
rem The "Safe" name doesn't have spaces or invalid characters and also doesn't change for Demo vs non-Demo
set ProjectName=Pig StdLib
set ProjectNameSafe=PigStdLib

set CompileLibrary=1
set CompileTest=1
set RunTest=0

rem Options: MSVC, GCC, Clang
set UseCompiler=GCC
rem Options: Native, Wasm
set TargetArch=Native
rem Options: Dll, Lib, Include
set LibFormat=Lib
set DebugBuild=1

rem Consider optional command-line arguments
if not "%1"=="" (
	set UseCompiler=%1
	rem Once any arguments are passed, local settings for what to compile are overridden
	set CompileLibrary=1
	set CompileTest=1
	set RunTest=0
)
if not "%2"=="" (
	set TargetArch=%2
)
if not "%3"=="" (
	set LibFormat=%3
)
if not "%4"=="" (
	set DebugBuild=%4
)
echo Compiler=%UseCompiler% Arch=%TargetArch% Format=%LibFormat% Debug=%DebugBuild%

set LibDirectory=..\lib
set SourceDirectory=..\source
set IncludeDirectory=..\include
set TestDirectory=..\test
set OutputDirectory=..\lib\release
set StdLibCodePath=%SourceDirectory%\unity_build.c
set TestCodePath=%TestDirectory%\main.c

echo Running on %ComputerName%

del *.dll > NUL 2> NUL
del *.lib > NUL 2> NUL
del *.pdb > NUL 2> NUL
del *.o > NUL 2> NUL
del *.obj > NUL 2> NUL
del *.ilk > NUL 2> NUL
del *.wat > NUL 2> NUL
del *.wasm > NUL 2> NUL

if "%UseCompiler%"=="MSVC" (
	build_msvc.bat
) else if "%UseCompiler%"=="GCC" (
	build_gcc.bat
) else if "%UseCompiler%"=="Clang" (
	build_clang.bat
) else (
	echo Unsupported compiler: %UseCompiler%
)
