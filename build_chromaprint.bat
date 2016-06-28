echo ---- Building chromaprint ----
set CHROMAPRINT_PATH=chromaprint-1.3.1

if %MACHINE_X86% (
  set PLATFORM=Win32
  rem No space after = below!
  set CMAKE_PLATFORM=
) else (
  set PLATFORM=x64
  rem Space between = and Win64 is intentional and required!
  set CMAKE_PLATFORM= Win64
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
erase cmakecache.txt
REM If CMAKE_PLATFORM is empty, CMake generates a Win32 version.
REM   No separating space is intentional
cmake . -G "Visual Studio 14 2015%CMAKE_PLATFORM%" -DWITH_FFTW3=ON -DFFTW3_DIR=../..

IF %STATIC_LIBS% (
  set PROJECT=chromaprint_p
) else (
  set PROJECT=chromaprint
)

%MSBUILD% chromaprint.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Clean;%PROJECT%:Rebuild

copy src\%CONFIG%\%PROJECT%.lib %LIB_DIR%
copy src\%CONFIG%\%PROJECT%.dll %LIB_DIR%
copy src\%PROJECT%.dir\%CONFIG%\%PROJECT%.pdb %LIB_DIR%

copy src\chromaprint.h %INCLUDE_DIR%

cd %ROOT_DIR%