echo ---- Building libsndfile ----
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
rem TODO(Pegasus): Need a way to specify FLAC__NO_DLL from here for static builds without clobbering the other defines in the project file
%MSBUILD% libsndfile.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libsndfile:Clean;libsndfile:Rebuild

copy %PLATFORM%\%CONFIG%\libsndfile.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libsndfile.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libsndfile.pdb %LIB_DIR%
copy src\sndfile.h %INCLUDE_DIR%

cd %ROOT_DIR%