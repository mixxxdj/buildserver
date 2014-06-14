echo "Building portmidi"
set PORTMIDI_PATH=portmidi-228

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

cd build\%PORTMIDI_PATH%\pm_dylib
%MSBUILD% portmidi-dynamic.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:portmidi-dynamic:Clean;portmidi-dynamic:Rebuild

copy %PLATFORM%\%CONFIG%\portmidi-dynamic.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\portmidi-dynamic.dll %LIB_DIR%
copy ..\pm_common\portmidi.h %INCLUDE_DIR% 

cd ..\porttime
%MSBUILD% porttime.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:PortTime:Clean;PortTime:Rebuild
copy %PLATFORM%\%CONFIG%\porttime.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\porttime.dll %LIB_DIR%
copy porttime.h %INCLUDE_DIR% 

cd %ROOT_DIR%