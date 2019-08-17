SETLOCAL
echo "Building libshout-idjc"
set SHOUT_PATH=libshout-idjc-2.4.1
SET VALRETURN=0

if %MACHINE_X86% (
  set PLATFORM=x86
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=Release
) else (
  set CONFIG=Debug
)

REM TODO(XXX): Fix the solution so the output path matches the platform name.
if %MACHINE_X86% (
  set OUTPUT_PATH=Win32\%CONFIG%
) else (
  set OUTPUT_PATH=%PLATFORM%\%CONFIG%
)

cd build\%SHOUT_PATH%

cd win32
%MSBUILD% libshout-idjc.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libshout-idjc:Clean;libshout-idjc:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy %OUTPUT_PATH%\libshout-idjc.lib %LIB_DIR%
if NOT %STATIC_LIBS% ( copy %OUTPUT_PATH%\libshout-idjc.dll %LIB_DIR% )
copy %OUTPUT_PATH%\libshout-idjc.pdb %LIB_DIR%
md %INCLUDE_DIR%\shoutidjc
copy ..\include\shoutidjc\*.h %INCLUDE_DIR%\shoutidjc\
copy ..\include\os.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%
