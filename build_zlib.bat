SETLOCAL
echo ---- Building Zlib ----
SET ZLIB_PATH=zlib-1.2.8
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

cd build\%ZLIB_PATH%\contrib\vstudio\vc14
echo Cleaning both...
%MSBUILD% zlibvc.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:zlibvc:Clean;zlibstat:Clean
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

echo Building...
if %STATIC_LIBS% (
  %MSBUILD% zlibvc.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:zlibstat:Rebuild
  IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
  )
  copy %MACHINE_X%\ZlibStat%CONFIG%\zlibwapi-static.lib %LIB_DIR%\zlibwapi.lib
  copy %MACHINE_X%\ZlibStat%CONFIG%\zlibwapi-static.pdb %LIB_DIR%\zlibwapi.pdb
  REM For QT4
  copy %MACHINE_X%\ZlibDll%CONFIG%\zlibwapi-static.lib %LIB_DIR%\zdll.lib

) else (
  %MSBUILD% zlibvc.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:zlibvc:Rebuild
  IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
  )
  copy %MACHINE_X%\ZlibDll%CONFIG%\zlibwapi.dll %LIB_DIR%
  copy %MACHINE_X%\ZlibDll%CONFIG%\zlibwapi.lib %LIB_DIR%
  copy %MACHINE_X%\ZlibDll%CONFIG%\zlibwapi.pdb %LIB_DIR%
  REM For QT4
  copy %MACHINE_X%\ZlibDll%CONFIG%\zlibwapi.lib %LIB_DIR%\zdll.lib
  copy %MACHINE_X%\ZlibDll%CONFIG%\zlibwapi.dll %BIN_DIR%
)

cd %BUILD_DIR%\%ZLIB_PATH%
copy zlib.h %INCLUDE_DIR%
copy zconf.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%