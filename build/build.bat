@echo off

rem NOTE: We need to change this in the build_installer.nsi as well
rem The "Safe" name doesn't have spaces or invalid characters and also doesn't change for Demo vs non-Demo
set ProjectName=Pig StdLib
set ProjectNameSafe=PigStdLib

set CompileLibrary=1
set CompileTest=1
set RunTest=1

rem Options: MSVC, GCC, Clang
set UseCompiler=MSVC
rem Options: Native, Wasm
set TargetArch=Native
set DebugBuild=1
set StaticLib=0

set LibDirectory=..\lib
set SourceDirectory=..\source
set IncludeDirectory=..\include
set TestDirectory=..\test
set OutputDirectory=..\lib\release
set StdLibCodePath=%SourceDirectory%\unity_build.c
set TestCodePath=%TestDirectory%\main.c

echo Running on %ComputerName%

if "%UseCompiler%"=="MSVC" (
	build_msvc.bat
) else if "%UseCompiler%"=="GCC" (
	build_gcc.bat
)
