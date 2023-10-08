@echo off

rem NOTE: We need to change this in the build_installer.nsi as well
rem The "Safe" name doesn't have spaces or invalid characters and also doesn't change for Demo vs non-Demo
set StdLibName=PigWasm StdLib
set StdLibNameSafe=PigWasmStdLib

set DebugBuild=1
set CopyToOutputDir=1
set ConvertToWat=1

set LibDirectory=..\lib
set SourceDirectory=..\source
set IncludeDirectory=..\include
set TestDirectory=..\source\test
set WebDirectory=..\source\web
set TestOutputDirectory=..\test
if "%DebugBuild%"=="1" (
	set LibOutputDirectory=%LibDirectory%\debug
) else (
	set LibOutputDirectory=%LibDirectory%\release
)
set StdMainCodePath=%SourceDirectory%\std_main.c
set TestCodePath=%TestDirectory%\main.cpp
set TestFileName=%StdLibNameSafe%_Test
set CombineFilesScript=%WebDirectory%\CombineFiles.py
set JavascriptFiles=%WebDirectory%\pig_wasm_std_lib.js
set JavascriptFiles=%JavascriptFiles% %TestDirectory%\main.js
set CombinedJsFileName=combined.js

echo Running on %ComputerName%

del *.wat > NUL 2> NUL
del *.wasm > NUL 2> NUL
del *.js > NUL 2> NUL
del *.o > NUL 2> NUL

rem --no-standard-libraries = ?
rem --no-standard-includes = ?
rem --target=wasm32 = ?
rem -mbulk-memory = Prevent conversion of simple loops into memset or memcpy?
rem -fno-builtin = (Optional) makes some calls like sqrtf actually go to our own sqrtf function rather than linking to the builtin clang implementation
set CompilerFlags=--no-standard-libraries --no-standard-includes --target=wasm32 -mbulk-memory
set IncludeDirectories=-I"%IncludeDirectory%" -I"%SourceDirectory%" -I"%LibDirectory%\include"
rem --no-entry        = ?
rem --allow-undefined = ?
rem --import-memory   = ?
rem --lto-O2          = ?
set LinkerFlags=--no-entry --allow-undefined --import-memory --lto-O2

if "%DebugBuild%"=="1" (
	set CompilerFlags=%CompilerFlags% -g -DSTD_ASSERTIONS_ENABLED
) else (
	set CompilerFlags=%CompilerFlags% -DSTD_ASSERTIONS_DISABLED
)

rem +--------------------------------------------------------------+
rem |                       Test Compilation                       |
rem +--------------------------------------------------------------+

echo[

clang "%TestCodePath%" -c %CompilerFlags% %IncludeDirectories% -o "%TestFileName%_Defines.txt" -dM -E

echo [Compiling...]
clang "%StdMainCodePath%" -c %CompilerFlags% %IncludeDirectories% -o "%StdLibNameSafe%.o"
clang "%TestCodePath%" -c %CompilerFlags% %IncludeDirectories% -o "%TestFileName%.o"
echo [Linking...]
wasm-ld "%StdLibNameSafe%.o" "%TestFileName%.o" %LinkerFlags% -o %TestFileName%.wasm

if "%ConvertToWat%"=="1" (
	echo [Creating %TestFileName%.wat]
	wasm2wat %TestFileName%.wasm > %TestFileName%.wat
)

python %CombineFilesScript% %CombinedJsFileName% %JavascriptFiles%

if "%CopyToOutputDir%"=="1" (
	echo [Copying %TestFileName%.wasm to %TestOutputDirectory%]
	XCOPY %TestFileName%.wasm "%TestOutputDirectory%\" /Y > NUL
	XCOPY %CombinedJsFileName% "%TestOutputDirectory%\" /Y > NUL
)
