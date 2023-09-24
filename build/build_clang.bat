@echo off

set OutputExt=dll
set OutputPdbName=%ProjectNameSafe%.pdb
if "%StaticLib%"=="1" (
	set OutputExt=lib
	set OutputPdbName=%ProjectNameSafe%_Static.pdb
)
if "%TargetArch%"=="Wasm" (
	set OutputExt=wasm
)

set CompilerFlags=-DOS_WINDOWS -DCOMPILER_CLANG -DPROJECT_NAME="\"%ProjectName%\"" -DPROJECT_NAME_SAFE=\"%ProjectNameSafe%\" -DDEBUG_BUILD=%DebugBuild%
set CompilerFlags=%CompilerFlags% -nostdlib -nostdinc
set CompilerFlags_Lib=-o "%ProjectNameSafe%.%OutputExt%"
set CompilerFlags_Test=-o "%ProjectNameSafe%_Test.exe" -lPigStdLib
set IncludeDirectories=-I"%IncludeDirectory%" -I"%SourceDirectory%" -I"%LibDirectory%\include\gylib"

if "%TargetArch%"=="Native" (
	set CompilerFlags=%CompilerFlags% -DARCH_NATIVE
) else if "%TargetArch%"=="Wasm" (
	set CompilerFlags=%CompilerFlags% -DARCH_WASM --target=wasm32 -mbulk-memory -Wl,--no-entry -Wl,--export-dynamic -Wl,--relocatable
) else (
	echo %TargetArch% is not supported with Clang compiler!
	EXIT
)

if "%DebugBuild%"=="1" (
	set CompilerFlags=%CompilerFlags% -g
	set OutputDirectory=%LibDirectory%\debug
) else (
	set CompilerFlags=%CompilerFlags%
	set OutputDirectory=%LibDirectory%\release
)

set LibraryDirectories=-L"%OutputDirectory%"

if "%StaticLib%"=="1" (
	set CompilerFlags_Lib=%CompilerFlags_Lib% -DSTD_FORMAT_LIB -static
) else (
	set CompilerFlags_Lib=%CompilerFlags_Lib% -DSTD_FORMAT_DLL -shared
)

if "%CompileLibrary%"=="1" (
	echo[
	
	del *.dll > NUL 2> NUL
	del *.lib > NUL 2> NUL
	del *.pdb > NUL 2> NUL
	del *.o > NUL 2> NUL
	del *.ilk > NUL 2> NUL
	del *.wasm > NUL 2> NUL
	del %OutputDirectory%\%OutputPdbName% > NUL 2> NUL
	del %OutputDirectory%\%ProjectNameSafe%.%OutputExt% > NUL 2> NUL
	
	clang %CompilerFlags% %CompilerFlags_Lib% %IncludeDirectories% "%StdLibCodePath%"
	
	if "%TargetArch%"=="Wasm" (
		if "%ConvertToWat%"=="1" (
			wasm2wat %ProjectNameSafe%.wasm > %ProjectNameSafe%.wat
		)
	)
	
	echo [Copying %ProjectNameSafe%.%OutputExt% to %OutputDirectory%]
	XCOPY ".\%ProjectNameSafe%.%OutputExt%" "%OutputDirectory%\" /Y > NUL
	if "%TargetArch%"=="Native" (
		if "%DebugBuild%"=="1" (
			XCOPY ".\%OutputPdbName%" "%OutputDirectory%\" /Y > NUL
		)
	)
	
)

if "%CompileTest%"=="1" (
	echo[
	
	clang %CompilerFlags% %CompilerFlags_Test% %IncludeDirectories% "%TestCodePath%" %LibraryDirectories%
)
