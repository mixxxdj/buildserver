REM Usage: build_environment.bat x86 Release
set MACHINE_X86="%1" == "x86"
set CONFIG_RELEASE="%2" == "Release"

if %MACHINE_X86% (
  echo Building for x86.
  set MACHINE_X=x86
) else (
  echo Building for x64.
  set MACHINE_X=x64
)

if %CONFIG_RELEASE% (
  echo Building release mode.
) else (
  echo Building debug mode.  
)

SET MSVC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC
SET XCOPY=xcopy /S /Y /I
SET MSBUILD=msbuild /p:VCTargetsPath="C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V120\\"
set ROOT_DIR=%CD%
SET BIN_DIR=%CD%\bin\
SET LIB_DIR=%CD%\lib\
SET INCLUDE_DIR=%CD%\include\
SET BUILD_DIR=%CD%\build\

set OLDPATH=%PATH%
if %MACHINE_X86% (
  call "%MSVC_PATH%\vcvarsall.bat" x86
) else (
  call "%MSVC_PATH%\vcvarsall.bat" x86_amd64
)

md %LIB_DIR%
md %INCLUDE_DIR%
md %BIN_DIR%

call build_sqlite3.bat
call build_zlib.bat 
call build_pthreads.bat
call build_protobuf.bat
call build_portmidi.bat
call build_libid3tag.bat REM depends on zlib
call build_libmad.bat
call build_libogg.bat
call build_libopus.bat REM depends on libogg
call build_libvorbis.bat
call build_libshout.bat
call build_libflac.bat
call build_libsndfile.bat
call build_rubberband.bat
call build_portaudio.bat
call build_hss1394.bat
call build_fftw3.bat
call build_chromaprint.bat REM depends on fftw3
call build_taglib.bat REM depends on zlib 
REM We do not distribute LAME with Mixxx. If you wish to build it locally, uncomment.
REM call build_lame.bat
call build_qt4.bat REM depends on sqlite3

REM Copy debug runtime DLLs for debug builds.
if not %CONFIG_RELEASE% (
  %XCOPY% "%MSVC_PATH%\redist\Debug_NonRedist\%MACHINE_X%\Microsoft.VC120.DebugCRT\*.dll" %LIB_DIR%
  %XCOPY% "%MSVC_PATH%\redist\Debug_NonRedist\%MACHINE_X%\Microsoft.VC120.DebugCXXAMP\*.dll" %LIB_DIR%
  %XCOPY% "%MSVC_PATH%\redist\Debug_NonRedist\%MACHINE_X%\Microsoft.VC120.DebugOpenMP\*.dll" %LIB_DIR%
)

REM Clean up after vcvarsall.bat since repeated running eventually overflows PATH.
SET PATH=%OLDPATH%