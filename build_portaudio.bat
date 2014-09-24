echo "Building portaudio"
set PORTAUDIO_PATH=pa_stable_v19_20140130

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

cd build\%PORTAUDIO_PATH%\build\msvc
%MSBUILD% portaudio.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:portaudio:Clean;portaudio:Rebuild

copy %PLATFORM%\%CONFIG%\portaudio.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\portaudio.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\portaudio.pdb %LIB_DIR%
copy ..\..\include\portaudio.h %INCLUDE_DIR%

cd %ROOT_DIR%