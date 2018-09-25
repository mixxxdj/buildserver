SETLOCAL
echo ---- Building FLAC ----
set FLAC_PATH=flac-1.3.2
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

set OLDFLACPATH=%PATH%
set PATH=%NASMPATH%;%PATH%

cd build\%FLAC_PATH%\
echo Cleaning both...
%MSBUILD% FLAC.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libFLAC_dynamic:Clean;libFLAC_static:Clean
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

echo Building...
if %STATIC_LIBS% (
  %MSBUILD% FLAC.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libFLAC_static:Rebuild
) else (
  %MSBUILD% FLAC.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libFLAC_dynamic:Rebuild
)
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)


set PATH=%OLDFLACPATH%
set OLDFLACPATH=

copy %PLATFORM%\%CONFIG%\libFLAC.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libFLAC.pdb %LIB_DIR%
if NOT %STATIC_LIBS% ( copy %PLATFORM%\%CONFIG%\libFLAC.dll %LIB_DIR% )
md %INCLUDE_DIR%\FLAC
copy include\FLAC\*.h %INCLUDE_DIR%\FLAC\

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%
