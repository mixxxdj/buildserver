echo "Building hss1394"
set HSS1394_PATH=hss1394-r8

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

cd build\%HSS1394_PATH%\code\builds\win32\VS2013
%MSBUILD% libHSS1394.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libHSS1394_dll:Clean;libHSS1394_dll:Rebuild

copy %PLATFORM%\%CONFIG%\libHSS1394.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libHSS1394.dll %LIB_DIR%
md %INCLUDE_DIR%\hss1394
copy ..\..\..\..\inc\*.h %INCLUDE_DIR%\hss1394\

cd %ROOT_DIR%