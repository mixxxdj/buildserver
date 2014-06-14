echo "Building zlib"
SET ZLIB_PATH=zlib-1.2.8

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=ReleaseWithoutAsm
) else (
  set CONFIG=Debug
)

cd build\%ZLIB_PATH%\contrib\vstudio\vc11
%MSBUILD% zlibvc.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:zlibvc:Clean;zlibvc:Rebuild

copy %MACHINE_X%\ZlibDll%CONFIG%\zlibwapi.dll %LIB_DIR%
copy %MACHINE_X%\ZlibDll%CONFIG%\zlibwapi.lib %LIB_DIR%
cd %BUILD_DIR%\%ZLIB_PATH%
copy zlib.h %INCLUDE_DIR%
copy zconf.h %INCLUDE_DIR%

cd %ROOT_DIR%