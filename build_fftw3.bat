echo ---- Building FFTW ----
set FFTW_PATH=fftw-3.3.4

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=Release
  if %STATIC_LIBS% ( 
    set CONFIG=Static-Release
  )
) else (
  set CONFIG=Debug
  if %STATIC_LIBS% ( 
    set CONFIG=Static-Debug
  )
)

cd build\%FFTW_PATH%\fftw-3.3-libs
rem cd build\%FFTW_PATH%\fftw-3.3-libs\

echo Cleaning all...
%MSBUILD% fftw-3.3-libs.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Clean

REM NOTE(rryan): Couldn't get solution itself to resolve the build actions. Odd.
cd libfftw-3.3
%MSBUILD% libfftw-3.3.vcxproj /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Rebuild
copy %PLATFORM%\%CONFIG%\libfftw-3.3.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libfftw-3.3.pdb %LIB_DIR%
if not %STATIC_LIBS% ( 
  copy %PLATFORM%\%CONFIG%\libfftw-3.3.dll %LIB_DIR%
) else (
  rem Need libfftwf as well
  cd ..\libfftwf-3.3
  %MSBUILD% libfftwf-3.3.vcxproj /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Rebuild
  copy %PLATFORM%\%CONFIG%\libfftwf-3.3.lib %LIB_DIR%
  copy %PLATFORM%\%CONFIG%\libfftwf-3.3.pdb %LIB_DIR%
)

copy ..\..\api\fftw3.h %INCLUDE_DIR%

cd %ROOT_DIR%