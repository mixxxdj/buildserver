echo "Building chromaprint"
set CHROMAPRINT_PATH=chromaprint-1.1

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

REM NOTE(rryan): generated solution with
REM C:\mixxx\environments\prototype\build\chromaprint-1.1>cmake . -G "Visual Studio 12 2013" -DWITH_FFTW3=ON -DFFTW3_FFTW_LIBRARY=c:/mixxx/environments/prototype/lib/libfftw-3.3.dll -DWITH_AVFFT=OFF -DBUILD_EXAMPLES=OFF

cd build\%CHROMAPRINT_PATH%
%MSBUILD% chromaprint.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:chromaprint:Clean;chromaprint:Rebuild

copy %PLATFORM%\%CONFIG%\chromaprint.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\chromaprint.dll %LIB_DIR%
copy src\chromaprint.h %INCLUDE_DIR%

cd %ROOT_DIR%