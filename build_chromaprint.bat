echo "Building chromaprint"
set CHROMAPRINT_PATH=chromaprint-1.1

if %MACHINE_X86% (
  set PLATFORM=Win32
  set CMAKE_CONF="Visual Studio 12 2013"
) else (
  set PLATFORM=x64
  set CMAKE_CONF="Visual Studio 12 2013 Win64"
)

if %CONFIG_RELEASE% (
  set CONFIG=Release
) else (
  set CONFIG=Debug
)

cd build\%CHROMAPRINT_PATH%
cmake . -G %CMAKE_CONF% -DWITH_FFTW3=ON -DFFTW3_FFTW_LIBRARY=%LIB_DIR%/libfftw-3.3.dll -DWITH_AVFFT=OFF -DBUILD_EXAMPLES=OFF -DFFTW3_DIR=../../include
%MSBUILD% chromaprint.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:chromaprint:Clean;chromaprint:Rebuild

copy %PLATFORM%\%CONFIG%\chromaprint.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\chromaprint.dll %LIB_DIR%
copy src\chromaprint.h %INCLUDE_DIR%

cd %ROOT_DIR%