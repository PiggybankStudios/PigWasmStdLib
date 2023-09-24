@echo off

set OutputExt=dll
set OutputPdbName=%ProjectNameSafe%.pdb
if "%StaticLib%"=="1" (
	set OutputExt=lib
	set OutputPdbName=%ProjectNameSafe%_Static.pdb
)

set CompilerFlags=-DOS_WINDOWS -DCOMPILER_GCC -DARCH_NATIVE -DPROJECT_NAME="\"%ProjectName%\"" -DPROJECT_NAME_SAFE=\"%ProjectNameSafe%\" -DDEBUG_BUILD=%DebugBuild%
set CompilerFlags=%CompilerFlags% -nostdlib
set CompilerFlags_Lib=-o "%ProjectNameSafe%.%OutputExt%"
set CompilerFlags_Test=-o "%ProjectNameSafe%_Test.exe" -lPigStdLib
set IncludeDirectories=-I"%IncludeDirectory%" -I"%SourceDirectory%" -I"%LibDirectory%\include\gylib"

if "%DebugBuild%"=="1" (
	set CompilerFlags=%CompilerFlags% -g
	set OutputDirectory=%LibDirectory%\debug
) else (
	set CompilerFlags=%CompilerFlags%
	set OutputDirectory=%LibDirectory%\debug
)

set LibraryDirectories=-L"%OutputDirectory%"

if "%StaticLib%"=="1" (
	set CompilerFlags_Lib=%CompilerFlags_Lib% -DSTD_FORMAT_LIB -static
) else (
	set CompilerFlags_Lib=%CompilerFlags_Lib% -DSTD_FORMAT_DLL -shared
)

if "%CompileLibrary%"=="1" (
	echo[
	
	del *.so > NUL 2> NUL
	del *.o > NUL 2> NUL
	del *.map > NUL 2> NUL
	del %OutputDirectory%\%OutputPdbName% > NUL 2> NUL
	del %OutputDirectory%\%ProjectNameSafe%.%OutputExt% > NUL 2> NUL
	
	gcc %CompilerFlags% %CompilerFlags_Lib% %IncludeDirectories% "%StdLibCodePath%"
	
	echo [Copying %ProjectNameSafe%.%OutputExt% to %OutputDirectory%]
	XCOPY ".\%ProjectNameSafe%.%OutputExt%" "%OutputDirectory%\" /Y > NUL
	if "%DebugBuild%"=="1" (
		XCOPY ".\%OutputPdbName%" "%OutputDirectory%\" /Y > NUL
	)
)

if "%CompileTest%"=="1" (
	echo[
	
	gcc %CompilerFlags% %CompilerFlags_Test% %IncludeDirectories% "%TestCodePath%" %LibraryDirectories%
)
