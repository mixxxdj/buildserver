echo "Building rubberband"
set RUBBERBAND_PATH=rubberband-1.8.1

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

cd build\%RUBBERBAND_PATH%
%MSBUILD% rubberband-library.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:rubberband-library:Clean;rubberband-library:Rebuild

copy %PLATFORM%\%CONFIG%\rubberband.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\rubberband.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\rubberband.pdb %LIB_DIR%
md %INCLUDE_DIR%\rubberband
copy rubberband\*.h %INCLUDE_DIR%\rubberband\

cd %ROOT_DIR%