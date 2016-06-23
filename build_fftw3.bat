echo ---- Building FFTW ----
set FFTW_PATH=fftw-3.3.4

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=Static-Release
) else (
  set CONFIG=Static-Debug
)

rem cd build\%FFTW_PATH%\fftw-3.3-libs
cd build\%FFTW_PATH%\fftw-3.3-libs\libfftw-3.3

REM TODO(Pegasus): Check for %STATIC_LIBS% and if not set, define FFTW_DLL somehow.

REM NOTE(rryan): Couldn't get solution itself to resolve the build actions. Odd.
rem %MSBUILD% fftw-3.3-libs.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libfftw-3.3:Clean;libfftw-3.3:Rebuild
%MSBUILD% libfftw-3.3.vcxproj /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Clean;Rebuild

copy %PLATFORM%\%CONFIG%\libfftw-3.3.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libfftw-3.3.pdb %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libfftw-3.3.dll %LIB_DIR%
copy ..\..\api\fftw3.h %INCLUDE_DIR%

cd %ROOT_DIR%