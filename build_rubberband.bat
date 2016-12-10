SETLOCAL
echo "Building rubberband"
set RUBBERBAND_PATH=rubberband-1.8.1
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

IF %STATIC_LIBS% (
  REM We have to set a value, so we set this one. It doesn't exist
  set FFTW_LINKING=FFTW__NO_DLL
) else (
  set FFTW_LINKING=FFTW_DLL
)


cd build\%RUBBERBAND_PATH%
%MSBUILD% rubberband-library.sln /p:TypeOfFftwLinking=%FFTW_LINKING% /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:rubberband-library:Clean;rubberband-library:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy %PLATFORM%\%CONFIG%\rubberband.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\rubberband.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\rubberband.pdb %LIB_DIR%
md %INCLUDE_DIR%\rubberband
copy rubberband\*.h %INCLUDE_DIR%\rubberband\

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%