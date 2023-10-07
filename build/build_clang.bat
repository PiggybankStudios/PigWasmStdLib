@echo off

set ConvertToWat=1

set OutputExt=
set OutputExeExt=exe
set OutputPdbName=%ProjectNameSafe%.pdb
if "%TargetArch%"=="Wasm" (
	set OutputExt=wasm
	set OutputExeExt=wasm
) else if "%LibFormat%"=="Lib" (
	set OutputExt=lib
	set OutputPdbName=%ProjectNameSafe%_Static.pdb
) else if "%LibFormat%"=="Dll" (
	set OutputExt=dll
) else if "%LibFormat%"=="Include" (
	rem
) else (
	echo %LibFormat% is not supported with Clang compiler!
	EXIT
)

set TestExeName=%ProjectNameSafe%_Test.%OutputExeExt%

set CompilerFlags=-DOS_WINDOWS -DCOMPILER_CLANG -DPROJECT_NAME="\"%ProjectName%\"" -DPROJECT_NAME_SAFE=\"%ProjectNameSafe%\" -DDEBUG_BUILD=%DebugBuild%
set CompilerFlags=%CompilerFlags% -nostdlib -nostdinc
set CompilerFlags_Lib=-o "%ProjectNameSafe%.%OutputExt%"
set CompilerFlags_Test=-o "%TestExeName%"
set IncludeDirectories=-I"%IncludeDirectory%" -I"%SourceDirectory%" -I"%LibDirectory%\include\gylib"
set LibraryDirectories=

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

if "%LibFormat%"=="Lib" (
	set CompilerFlags_Lib=%CompilerFlags_Lib% -DSTD_FORMAT_LIB -static
	set CompilerFlags_Test=%CompilerFlags_Test% -lPigStdLib
	set LibraryDirectories=%LibraryDirectories% -L"%OutputDirectory%"
) else if "%LibFormat%"=="Dll" (
	set CompilerFlags_Lib=%CompilerFlags_Lib% -DSTD_FORMAT_DLL -shared
	rem TODO: How does the .dll version get used by an application?
) else if "%LibFormat%"=="Include" (
	set CompilerFlags_Test=%CompilerFlags_Test% -DINCLUDE_STD_LIBRARY -DSTD_FORMAT_INCLUDE
)

if "%CompileLibrary%"=="1" (
	if "%LibFormat%"=="Include" (
		echo skipping library build
	) else (
		echo[
		
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
)

if "%CompileTest%"=="1" (
	echo[
	
	clang %CompilerFlags% %CompilerFlags_Test% %IncludeDirectories% "%TestCodePath%" %LibraryDirectories%
	
	if "%TargetArch%"=="Wasm" (
		if "%ConvertToWat%"=="1" (
			wasm2wat %TestExeName% > %ProjectNameSafe%_Test.wat
		)
	)
)

if "%RunTest%"=="1" (
	echo [Running %TestExeName%...]
	%TestExeName%
	echo [Finished!]
)
