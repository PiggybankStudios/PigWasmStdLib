@echo off

rem NOTE: We need to change this in the build_installer.nsi as well
rem The "Safe" name doesn't have spaces or invalid characters and also doesn't change for Demo vs non-Demo
set StdLibName=PigWasm StdLib
set StdLibNameSafe=PigWasmStdLib

set DebugBuild=1
set ConvertToWat=1

set LibDirectory=..\lib
set SourceDirectory=..\source
set IncludeDirectory=..\include
set TestDirectory=..\test
if "%DebugBuild%"=="1" (
	set OutputDirectory=%LibDirectory%\debug
) else (
	set OutputDirectory=%LibDirectory%\release
)
set StdMainCodePath=%SourceDirectory%\std_main.c
set TestCodePath=%TestDirectory%\main.c
set TestFileName=%StdLibNameSafe%_Test

echo Running on %ComputerName%

del *.o > NUL 2> NUL
del *.wat > NUL 2> NUL
del *.wasm > NUL 2> NUL

set CompilerFlags=-DSTD_LIB_NAME="\"%StdLibNameName%\"" -DSTD_LIB_NAME_SAFE=\"%StdLibNameNameSafe%\" -DSTD_DEBUG_BUILD=%DebugBuild%
set CompilerFlags=%CompilerFlags% -nostdlib -nostdinc --target=wasm32 -mbulk-memory -Wl,--no-entry -Wl,--export-dynamic -Wl,--relocatable
set IncludeDirectories=-I"%IncludeDirectory%" -I"%SourceDirectory%" -I"%LibDirectory%\include"
set LibraryDirectories=

if "%DebugBuild%"=="1" (
	set CompilerFlags=%CompilerFlags% -g
) else (
	set CompilerFlags=%CompilerFlags%
)

echo[

clang -o "%TestFileName%.wasm" -DTEST_INCLUDE_STD_LIBRARY %CompilerFlags% %IncludeDirectories% "%TestCodePath%" %LibraryDirectories%

if "%ConvertToWat%"=="1" (
	wasm2wat %TestFileName%.wasm > %TestFileName%.wat
)
