echo "Building opus"
set OPUS_PATH=opus-1.1
set OPUSFILE_PATH=opusfile-0.6

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

cd build\%OPUS_PATH%\win32\VS2010
%MSBUILD% opus.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:celt:Clean;silk_common:Clean;silk_fixed:Clean;silk_float:Clean;opus:Clean;opus:Rebuild

copy %PLATFORM%\%CONFIG%\opus.lib %LIB_DIR%
md %INCLUDE_DIR%\opus
copy ..\..\include\*.h %INCLUDE_DIR%\opus\

cd %ROOT_DIR%

cd build\%OPUSFILE_PATH%\win32\VS2010
%MSBUILD% opusfile.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:opusfile:Clean;opusfile:Rebuild

copy %PLATFORM%\%CONFIG%\opusfile.lib %LIB_DIR%
md %INCLUDE_DIR%\opus
copy ..\..\include\*.h %INCLUDE_DIR%\opus\

cd %ROOT_DIR%