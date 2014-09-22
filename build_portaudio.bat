echo "Building portaudio"
set PORTAUDIO_PATH=pa_stable_v19_20140130

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

cd build\%PORTAUDIO_PATH%
cmake . -G %CMAKE_CONF% -DPA_USE_ASIO=OFF -DPA_WDMKS_NO_KSGUID_LIB=ON
%MSBUILD% portaudio.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:portaudio:Clean;portaudio:Rebuild

copy %CONFIG%\portaudio_%PLATFORM%.lib %LIB_DIR%\portaudio.lib
copy %CONFIG%\portaudio_%PLATFORM%.dll %LIB_DIR%\portaudio.dll
copy include\portaudio.h %INCLUDE_DIR%

cd %ROOT_DIR%