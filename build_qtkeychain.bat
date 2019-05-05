SETLOCAL
echo ---- Building qtkeychain ----
set QTKEYCHAIN_PATH=qtkeychain-0.9.1
set QT_VERSION=5.12.3

set VALRETURN=0

if %MACHINE_X86% (
  set PLATFORM=Win32
  set COMPILERPROJECT="Visual Studio 15 2017"
) else (
  set PLATFORM=x64
  set COMPILERPROJECT="Visual Studio 15 2017 Win64"
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

    
"%CMAKEDIR%\cmake" .. -G %COMPILERPROJECT% -T %PLATFORM_TOOLSET% %ADDITIONAL_SETTINGS% -DBUILD_TEST_APPLICATION=OFF -DCMAKE_PREFIX_PATH=%ROOT_DIR%\Qt-%QT_VERSION%\lib\cmake -DCMAKE_INSTALL_PREFIX=%ROOT_DIR%
IF ERRORLEVEL 1 (
    SET VALRETURN=1
    goto END
)

"%CMAKEDIR%\cmake" --build . --config %CONFIG% --clean-first 
IF ERRORLEVEL 1 (
    SET VALRETURN=1
    goto END
)

"%CMAKEDIR%\cmake" --build . --config %CONFIG% --target install 
IF ERRORLEVEL 1 (
    SET VALRETURN=1
    goto END
)

copy qt5keychain.dir\%CONFIG%\qt5keychain.pdb %LIB_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%
