SETLOCAL
echo ---- Building chromaprint ----
set CHROMAPRINT_PATH=chromaprint-1.3.1
SET VALRETURN=0

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
erase %LIB_DIR%\*chromaprint*

cd build\%CHROMAPRINT_PATH%
REM Re-generate solution files.
rd /S /Q %PLATFORM%
mkdir %PLATFORM%
cd %PLATFORM%

REM TODO: verify that the cmake setting BUILD_SHARED_LIBS works correctly.
SET ADDITIONAL_SETTINGS=
IF NOT %STATIC_LIBS% (
SET ADDITIONAL_SETTINGS=-DBUILD_SHARED_LIBS=ON -DWITH_FFTW3_DYNAMIC=ON
)
"%CMAKEDIR%\cmake" ../ -G %COMPILERPROJECT% %ADDITIONAL_SETTINGS% -DWITH_FFTW3=ON -DFFTW3_DIR=../../..
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

IF %STATIC_LIBS% (
  set PROJECT=chromaprint_p
) else (
  set PROJECT=chromaprint
)

%MSBUILD% chromaprint.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Clean;%PROJECT%:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy src\%CONFIG%\%PROJECT%.lib %LIB_DIR%
copy src\%CONFIG%\%PROJECT%.dll %LIB_DIR%
copy src\%CONFIG%\%PROJECT%.pdb %LIB_DIR%
cd ..
copy src\chromaprint.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%