SETLOCAL
echo ---- Building libsndfile ----
set SNDFILE_PATH=libsndfile-1.0.26
SET VALRETURN=0

if %CONFIG_RELEASE% (
  set CONFIG=Static-Release
) else (
  set CONFIG=Static-Debug
)

if %MACHINE_X86% (
  set PLATFORM=Win32
  REM TODO(rryan): Fix this in the solution.
  set LIB_OUTPUT_PATH=%CONFIG%
) else (
  set PLATFORM=x64
  set LIB_OUTPUT_PATH=%PLATFORM%\%CONFIG%
)

IF %STATIC_LIBS% (
  set FLAC_LINKING=FLAC__NO_DLL
) else (
  REM We have to set a value, so we set this one. It doesn't exist
  set FLAC_LINKING=FLAC__DLL
)


cd build\%SNDFILE_PATH%
%MSBUILD% libsndfile.sln /p:TypeOfFlacLinking=%FLAC_LINKING% /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libsndfile:Rebuild
IF ERRORLEVEL 1 (
	SET VALRETURN=1
	goto END
)

rem If built statically (and right now we always build this statically) Symbols from this file are not available from libsndfile
copy %PLATFORM%\%CONFIG%\g72x.lib %LIB_DIR%
copy %LIB_OUTPUT_PATH%\libsndfile.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libsndfile.pdb %LIB_DIR%

IF NOT %STATIC_LIBS% ( copy %PLATFORM%\%CONFIG%\libsndfile.dll %LIB_DIR% )

copy src\sndfile.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%
