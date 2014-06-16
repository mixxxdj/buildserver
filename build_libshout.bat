echo "Building libshout"
set SHOUT_PATH=libshout-2.3.1

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

cd build\%SHOUT_PATH%\win32
%MSBUILD% libshout.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libshout:Clean;libshout:Rebuild

copy %PLATFORM%\%CONFIG%\libshout.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libshout.dll %LIB_DIR%
md %INCLUDE_DIR%\shout
copy ..\include\shout\*.h %INCLUDE_DIR%\shout\
copy ..\include\os.h %INCLUDE_DIR%

cd %ROOT_DIR%