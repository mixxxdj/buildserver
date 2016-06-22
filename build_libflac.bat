echo ---- Building FLAC ----
set FLAC_PATH=flac-1.3.1

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

set OLDFLACPATH=%PATH%
set PATH=E:\Mixxx\Buildserver\build\nasm-2.11.08;%PATH%

cd build\%FLAC_PATH%\src\libFLAC\
echo Cleaning both...
%MSBUILD% libFLAC_dynamic.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libFLAC_dynamic:Clean
%MSBUILD% libFLAC_static.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libFLAC:Clean

echo Building...
if %STATIC_LIBS% (
  %MSBUILD% libFLAC_static.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libFLAC:Clean
) else (
  %MSBUILD% libFLAC_dynamic.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libFLAC_dynamic:Rebuild
)


set PATH=%OLDFLACPATH%
set OLDFLACPATH=

copy %PLATFORM%\%CONFIG%\libFLAC.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libFLAC.pdb %LIB_DIR%
if NOT %STATIC_LIBS% ( copy %PLATFORM%\%CONFIG%\libFLAC.dll %LIB_DIR% )
md %INCLUDE_DIR%\FLAC
copy ..\..\include\FLAC\*.h %INCLUDE_DIR%\FLAC\

cd %ROOT_DIR%