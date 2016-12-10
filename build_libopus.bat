SETLOCAL
echo ---- Building Opus ----
set OPUS_PATH=opus-1.1.2
set OPUSFILE_PATH=opusfile-0.7
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

cd build\%OPUS_PATH%\win32\VS2015
%MSBUILD% opus.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Clean;opus:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy %PLATFORM%\%CONFIG%\opus.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\opus.pdb %LIB_DIR%
copy %PLATFORM%\%CONFIG%\opus.dll %LIB_DIR%
md %INCLUDE_DIR%\opus
copy ..\..\include\*.h %INCLUDE_DIR%\opus\

cd %ROOT_DIR%

cd build\%OPUSFILE_PATH%\win32\VS2015
%MSBUILD% opusfile.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:opusfile:Clean;opusfile:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy %PLATFORM%\%CONFIG%\opusfile.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\opusfile.pdb %LIB_DIR%
copy %PLATFORM%\%CONFIG%\opusfile.dll %LIB_DIR%
copy ..\..\include\*.h %INCLUDE_DIR%\opus\

rem Fix a path problem
REM NOTE: We need forward slashes for python to not crash on some paths
SETLOCAL ENABLEDELAYEDEXPANSION
for /f "delims=" %%i in ("%INCLUDE_DIR%") do (
    set "replaceWith=/"
    set "str=%%i"
    call set "str=%%str:\=!replaceWith!%%"
)
ENDLOCAL & SET INCLUDE_DIR_FOR_PYTHON=%str%

python -c "import sys; sys.stdout.write(open('%INCLUDE_DIR_FOR_PYTHON%opus/opusfile.h', 'r').read().replace('opus_multistream.h', 'opus/opus_multistream.h'))" > temp
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)
copy temp %INCLUDE_DIR%\opus\opusfile.h
erase temp

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%