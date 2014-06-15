echo "Building libogg"
set OGG_PATH=libogg-1.3.2

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

cd build\%OGG_PATH%\win32\VS2010
%MSBUILD% libogg_dynamic.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libogg:Clean;libogg:Rebuild

copy %PLATFORM%\%CONFIG%\libogg.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libogg.dll %LIB_DIR%
md %INCLUDE_DIR%\ogg
copy ..\..\include\ogg\*.h %INCLUDE_DIR%\ogg\

cd %ROOT_DIR%