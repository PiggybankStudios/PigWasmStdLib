@echo off

del *.pdb > NUL 2> NUL

if "%CompileLibrary%"=="1" (
	echo[
	
	gcc "%StdLibCodePath%"
	
	echo [Copying %ProjectNameSafe%.dll/lib to %OutputDirectory%]
	XCOPY ".\%ProjectNameSafe%.dll" "%OutputDirectory%\" /Y > NUL
	XCOPY ".\%ProjectNameSafe%.lib" "%OutputDirectory%\" /Y > NUL
	XCOPY ".\%ProjectNameSafe%.pdb" "%OutputDirectory%\" /Y > NUL
)

if "%CompileTest%"=="1" (
	echo[
	
	gcc "%TestCodePath%" -nostdlib -nodefaultlibs
)
