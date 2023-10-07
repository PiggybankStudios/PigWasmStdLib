@echo off

if "%TargetArch%"=="Native" (
	rem
) else (
	echo %TargetArch% is not supported with GCC compiler!
	EXIT
)

set OutputExt=
set OutputPdbName=%ProjectNameSafe%.pdb
if "%LibFormat%"=="Lib" (
	set OutputExt=lib
	set OutputPdbName=%ProjectNameSafe%_Static.pdb
) else if "%LibFormat%"=="Dll" (
	set OutputExt=dll
) else if "%LibFormat%"=="Include" (
	rem
) else (
	echo %LibFormat% is not supported with GCC compiler!
	EXIT
)

set TestExeName=%ProjectNameSafe%_Test.exe

set CompilerFlags=-DOS_WINDOWS -DCOMPILER_GCC -DARCH_NATIVE -DPROJECT_NAME="\"%ProjectName%\"" -DPROJECT_NAME_SAFE=\"%ProjectNameSafe%\" -DDEBUG_BUILD=%DebugBuild%
set CompilerFlags=%CompilerFlags% -nostdlib --no-standard-includes
set CompilerFlags_Lib=-o "%ProjectNameSafe%.%OutputExt%"
set CompilerFlags_Test=-o "%TestExeName%"
set IncludeDirectories=-I"%IncludeDirectory%" -I"%SourceDirectory%" -I"%LibDirectory%\include\gylib"
set LibraryDirectories=

if "%DebugBuild%"=="1" (
	set CompilerFlags=%CompilerFlags% -g
	set OutputDirectory=%LibDirectory%\debug
) else (
	set CompilerFlags=%CompilerFlags%
	set OutputDirectory=%LibDirectory%\release
)

if "%LibFormat%"=="Lib" (
	set CompilerFlags_Lib=%CompilerFlags_Lib% -DSTD_FORMAT_LIB -static -fPIC
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
		
		gcc %CompilerFlags% %CompilerFlags_Lib% %IncludeDirectories% "%StdLibCodePath%"
		
		echo [Copying %ProjectNameSafe%.%OutputExt% to %OutputDirectory%]
		XCOPY ".\%ProjectNameSafe%.%OutputExt%" "%OutputDirectory%\" /Y > NUL
		if "%DebugBuild%"=="1" (
			XCOPY ".\%OutputPdbName%" "%OutputDirectory%\" /Y > NUL
		)
	)
)

if "%CompileTest%"=="1" (
	echo[
	
	gcc %CompilerFlags% %CompilerFlags_Test% %IncludeDirectories% "%TestCodePath%" %LibraryDirectories%
)

if "%RunTest%"=="1" (
	echo [Running %TestExeName%...]
	%TestExeName%
	echo [Finished!]
)
