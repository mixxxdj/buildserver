echo "Building chromaprint"
set CHROMAPRINT_PATH=chromaprint-1.3.1

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=RelWithDebInfo
) else (
  set CONFIG=Debug
)

REM NOTE(pegasus): generated solution one with
REM E:\Mixxx\Buildserver\build\chromaprint-1.3.1>cmake . -G "Visual Studio 14 2015 Win64" -DWITH_FFTW3=ON -DFFTW3_DIR=../..

cd build\%CHROMAPRINT_PATH%
%MSBUILD% chromaprint.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:chromaprint:Clean;chromaprint:Rebuild

copy %PLATFORM%\%CONFIG%\chromaprint.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\chromaprint.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\chromaprint.pdb %LIB_DIR%
copy src\chromaprint.h %INCLUDE_DIR%

cd %ROOT_DIR%