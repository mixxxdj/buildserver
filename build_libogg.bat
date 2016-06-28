echo ---- Building OGG ----
set OGG_PATH=ogg-1.3.2

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

cd build\%OGG_PATH%\win32\VS2015
echo Cleaning both...
%MSBUILD% libogg_dynamic.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libogg:Clean
%MSBUILD% libogg_static.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Clean

echo Building...
if %STATIC_LIBS% (
  %MSBUILD% libogg_static.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Rebuild
) else (
  %MSBUILD% libogg_dynamic.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libogg:Rebuild
)

copy %PLATFORM%\%CONFIG%\libogg.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libogg.pdb %LIB_DIR%
if NOT %STATIC_LIBS% ( copy %PLATFORM%\%CONFIG%\libogg.dll %LIB_DIR% )
md %INCLUDE_DIR%\ogg
copy ..\..\include\ogg\*.h %INCLUDE_DIR%\ogg\

cd %ROOT_DIR%