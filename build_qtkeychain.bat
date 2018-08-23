SETLOCAL
echo ---- Building qtkeychain ----
set QTKEYCHAIN_PATH=qtkeychain-0.9.1
set QT_VERSION=5.11.1

set VALRETURN=0

if %MACHINE_X86% (
  set PLATFORM=Win32
  set COMPILERPROJECT="Visual Studio 14 2015"
) else (
  set PLATFORM=x64
  set COMPILERPROJECT="Visual Studio 14 2015 Win64"
)

if %CONFIG_RELEASE% (
  set CONFIG=RelWithDebInfo
) else (
  set CONFIG=Debug
)

REM Remove any existing files
erase %LIB_DIR%\*qtkeychain*

cd build\%QTKEYCHAIN_PATH%
REM Re-generate solution files.
rd /S /Q %PLATFORM%
mkdir %PLATFORM%
cd %PLATFORM%

REM TODO: verify that the cmake setting BUILD_SHARED_LIBS works correctly.
SET ADDITIONAL_SETTINGS=
IF %STATIC_LIBS% (
SET ADDITIONAL_SETTINGS=-DQTKEYCHAIN_STATIC=ON
)

"%CMAKEDIR%\cmake" ../ -G %COMPILERPROJECT% %ADDITIONAL_SETTINGS% -DBUILD_TEST_APPLICATION=OFF -DCMAKE_PREFIX_PATH=%ROOT_DIR%\Qt-%QT_VERSION%\lib\cmake
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

IF %STATIC_LIBS% (
  set PROJECT=qtkeychain_p
) else (
  set PROJECT=qtkeychain
)

%MSBUILD% qtkeychain.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Clean;%PROJECT%:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy src\%CONFIG%\%PROJECT%.lib %LIB_DIR%
IF NOT %STATIC_LIBS% ( copy src\%CONFIG%\%PROJECT%.dll %LIB_DIR% )
copy src\%CONFIG%\%PROJECT%.pdb %LIB_DIR%
cd ..
copy keychain.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%
