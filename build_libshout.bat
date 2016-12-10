SETLOCAL
echo "Building libshout"
set SHOUT_PATH=libshout-2.4.1
SET VALRETURN=0

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=Release
) else (
  set CONFIG=Debug
)

cd build\%SHOUT_PATH%\win32
%MSBUILD% libshout.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libshout:Clean;libshout:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy %PLATFORM%\%CONFIG%\libshout.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libshout.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libshout.pdb %LIB_DIR%
md %INCLUDE_DIR%\shout
copy ..\include\shout\*.h %INCLUDE_DIR%\shout\
copy ..\include\os.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%