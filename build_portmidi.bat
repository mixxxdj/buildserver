echo ---- Building PortMIDI ----
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

cd build\%PORTMIDI_PATH%
echo Cleaning both...
%MSBUILD% portmidi.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:portmidi-dynamic:Clean
%MSBUILD% portmidi.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:portmidi-static:Clean

echo Building...
if %STATIC_LIBS% (
  %MSBUILD% portmidi.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:portmidi-static:Rebuild
) else (
  %MSBUILD% portmidi.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:portmidi-dynamic:Rebuild
)

copy %PLATFORM%\%CONFIG%\portmidi.lib %LIB_DIR%
if NOT %STATIC_LIBS% ( copy %PLATFORM%\%CONFIG%\portmidi.dll %LIB_DIR% )
copy %PLATFORM%\%CONFIG%\portmidi.pdb %LIB_DIR%
copy ..\pm_common\portmidi.h %INCLUDE_DIR% 

cd porttime
%MSBUILD% porttime.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:PortTime:Clean;PortTime:Rebuild
copy %PLATFORM%\%CONFIG%\porttime.lib %LIB_DIR%
if NOT %STATIC_LIBS% ( copy %PLATFORM%\%CONFIG%\porttime.dll %LIB_DIR% )
copy %PLATFORM%\%CONFIG%\porttime.pdb %LIB_DIR%
copy porttime.h %INCLUDE_DIR% 

cd %ROOT_DIR%