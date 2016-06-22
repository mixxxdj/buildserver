echo "Building libvorbis"
set VORBIS_PATH=vorbis

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

cd build\%VORBIS_PATH%\win32\VS2015
rem %MSBUILD% vorbis_dynamic.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libvorbis:Clean;libvorbis:Rebuild;libvorbisfile:Clean;libvorbisfile:Rebuild
rem Need to rename the libvorbis_static project to just libvorbis first.
%MSBUILD% vorbis_static.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libvorbis_static:Clean;libvorbis_static:Rebuild;libvorbisfile:Clean;libvorbisfile:Rebuild

copy %PLATFORM%\%CONFIG%\libvorbis.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libvorbis.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libvorbis.pdb %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libvorbisfile.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libvorbisfile.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libvorbisfile.pdb %LIB_DIR%
md %INCLUDE_DIR%\vorbis
copy ..\..\include\vorbis\*.h %INCLUDE_DIR%\vorbis\

cd %ROOT_DIR%