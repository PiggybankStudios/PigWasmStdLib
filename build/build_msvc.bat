@echo off

if "%TargetArch%"=="Native" (
	rem
) else (
	echo %TargetArch% is not supported with MSVC compiler!
	EXIT
)

rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
rem call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64

set OutputExt=dll
set OutputPdbName=%ProjectNameSafe%.pdb
if "%StaticLib%"=="1" (
	set OutputExt=lib
	set OutputPdbName=%ProjectNameSafe%_Static.pdb
)
set TestExeName=%ProjectNameSafe%_Test.exe

set CompilerFlags=/DOS_WINDOWS /DCOMPILER_MSVC /DARCH_NATIVE /DPROJECT_NAME="\"%ProjectName%\"" /DPROJECT_NAME_SAFE=\"%ProjectNameSafe%\" /DDEBUG_BUILD=%DebugBuild%
rem /FC = Full path for error messages
rem /EHsc = Exception Handling Model: Standard C++ Stack Unwinding. Functions declared as extern "C" can't throw exceptions
rem /EHa- = TODO: Do we really need this?? It seems like this option should be off if we specify s and c earlier
rem /nologo = Suppress the startup banner
rem /GS- = Buffer overrun protection is turned off
rem /Gm- = Minimal rebuild is enabled [deprecated]
rem /GR- = Run-Time Type Information is Disabled [_CPPRTTI macro doesn't work]
rem /Fm = Enables map file output [with default name, same as /MAP linker option]
rem /Oi = Enable intrinsic generation
rem /WX = Treat warnings as errors
rem /W4 = Warning level 4 [just below /Wall]
rem /X = Ignore standard include directories
set CompilerFlags=%CompilerFlags% /FC /EHsc /EHa- /nologo /GS- /Gm- /GR- /Fm /Oi /WX /W4 /X
rem /wd4146 = unary minus operator applied to unsigned type, result still unsigned
set CompilerFlags=%CompilerFlags% /wd4146
set CompilerFlags_Lib=/Fe%ProjectNameSafe%.%OutputExt% /Fd"%OutputPdbName%"
set CompilerFlags_Test=/Fe%TestExeName%
rem /NODEFAULTLIB = ?
rem /SUBSYSTEM:windows = ?
set LinkerFlags=/NODEFAULTLIB /NOLOGO /SUBSYSTEM:windows
set LinkerFlags_Lib=
set LinkerFlags_Test=%ProjectNameSafe%.lib
set IncludeDirectories=/I"%IncludeDirectory%" /I"%SourceDirectory%" /I"%LibDirectory%\include\gylib"

if "%DebugBuild%"=="1" (
	rem /Od = Optimization level: Debug
	rem /Zi = Generate complete debugging information
	set CompilerFlags=%CompilerFlags% /Od /Zi
	set OutputDirectory=%LibDirectory%\debug
) else (
	rem /Ot = Favors fast code over small code
	rem /Oy = Omit frame pointer [x86 only]
	rem /O2 = Optimization level 2: Creates fast code
	rem /Zi = Generate complete debugging information [optional]
	set CompilerFlags=%CompilerFlags% /Ot /Oy /O2
	set OutputDirectory=%LibDirectory%\release
)

set LinkerFlags_Test=%LinkerFlags_Test% /LIBPATH:"%OutputDirectory%"

if "%StaticLib%"=="1" (
	set CompilerFlags_Lib=%CompilerFlags_Lib% /DSTD_FORMAT_LIB
	set LinkerFlags_Lib=%LinkerFlags_Lib% /OUT:"%ProjectNameSafe%.%OutputExt%"
) else (
	set CompilerFlags_Lib=%CompilerFlags_Lib% /DSTD_FORMAT_DLL
	set LinkerFlags_Lib=%LinkerFlags_Lib% /DLL /NOENTRY
)

if "%CompileLibrary%"=="1" (
	echo[
	
	del *.pdb > NUL 2> NUL
	del *.dll > NUL 2> NUL
	del *.lib > NUL 2> NUL
	del *.obj > NUL 2> NUL
	del *.ilk > NUL 2> NUL
	del *.map > NUL 2> NUL
	del %OutputDirectory%\%OutputPdbName% > NUL 2> NUL
	del %OutputDirectory%\%ProjectNameSafe%.%OutputExt% > NUL 2> NUL
	
	if "%StaticLib%"=="1" (
		cl /c /Fo"%ProjectNameSafe%.obj" %CompilerFlags% %CompilerFlags_Lib% %IncludeDirectories% "%StdLibCodePath%"
		IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
		link /lib %ProjectNameSafe%.obj %LinkerFlags% %LinkerFlags_Lib%
		IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
	) else (
		cl %CompilerFlags% %CompilerFlags_Lib% %IncludeDirectories% "%StdLibCodePath%" /link %LinkerFlags% %LinkerFlags_Lib%
		IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
	)
	
	echo [Copying %ProjectNameSafe%.%OutputExt% to %OutputDirectory%]
	XCOPY ".\%ProjectNameSafe%.%OutputExt%" "%OutputDirectory%\" /Y > NUL
	if "%DebugBuild%"=="1" (
		XCOPY ".\%OutputPdbName%" "%OutputDirectory%\" /Y > NUL
	)
)

if "%CompileTest%"=="1" (
	echo[
	
	cl %CompilerFlags% %CompilerFlags_Test% %IncludeDirectories% "%TestCodePath%" /link %LinkerFlags% %LinkerFlags_Test%
	IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
)

if "%RunTest%"=="1" (
	echo [Running %TestExeName%...]
	%TestExeName%
	echo [Finished!]
)
