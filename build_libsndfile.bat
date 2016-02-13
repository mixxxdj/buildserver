echo "Building libsndfile"
set SNDFILE_PATH=libsndfile-1.0.26

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

cd build\%SNDFILE_PATH%
%MSBUILD% libsndfile.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libsndfile:Clean;libsndfile:Rebuild

copy %PLATFORM%\%CONFIG%\libsndfile.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libsndfile.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libsndfile.pdb %LIB_DIR%
copy src\sndfile.h %INCLUDE_DIR%

cd %ROOT_DIR%