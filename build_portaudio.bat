SETLOCAL
echo "Building portaudio"
set PORTAUDIO_PATH=pa_stable_v19_20140130
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

cd build\%PORTAUDIO_PATH%\build\msvc
%MSBUILD% portaudio.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:portaudio:Clean;portaudio:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy %PLATFORM%\%CONFIG%\portaudio.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\portaudio.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\portaudio.pdb %LIB_DIR%
copy ..\..\include\portaudio.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%