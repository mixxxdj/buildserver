SETLOCAL
echo "Building lame"
set LAME_PATH=lame-3.100
SET VALRETURN=0

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=ReleaseSSE2
) else (
  set CONFIG=Debug
)

cd build\%LAME_PATH%

%MSBUILD% vc_solution\vc9_lame.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libmp3lame:Clean;libmp3lame:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

if not %STATIC_LIBS% (
  copy output\%CONFIG%\libmp3lame.lib %LIB_DIR%
  copy output\%CONFIG%\libmp3lame.dll %LIB_DIR%
) else (
  copy output\%CONFIG%\libmp3lame-static.lib %LIB_DIR%
)
copy output\%CONFIG%\libmp3lame.pdb %LIB_DIR%
copy include\lame.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%
