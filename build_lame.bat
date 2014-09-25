echo "Building lame"
set LAME_PATH=lame-3.99.5

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=ReleaseSSE2
) else (
  set CONFIG=Debug
)

cd build\%LAME_PATH%

%MSBUILD% vc_solution\vc9_lame.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libmp3lame:Clean;libmp3lame:Rebuild

copy output\%CONFIG%\libmp3lame.lib %LIB_DIR%
copy output\%CONFIG%\libmp3lame.dll %LIB_DIR%
copy output\%CONFIG%\libmp3lame.pdb %LIB_DIR%
copy include\lame.h %INCLUDE_DIR%

cd %ROOT_DIR%