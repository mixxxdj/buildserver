@ECHO off
REM Usage: build_environment.bat x86 Release [nuke]

set MACHINE_X86="%1" == "x86"
set CONFIG_RELEASE="%2" == "Release"
set DYNAMIC_LIBS="%3" == "Dynamic"

if %MACHINE_X86% (
  echo Building for x86.
  set MACHINE_X=x86
) else (
  echo Building for x64.
  set MACHINE_X=x64
)

if %CONFIG_RELEASE% (
  echo Building release mode.
  set RUNTIMELIB="MultiThreadedDLL"
  REM the above doesn't actually seem to work, so we do the following
  set _CL_=/MD
) else (
  echo Building debug mode.
  set RUNTIMELIB="MultiThreadedDebugDLL"
  set _CL_=/MDd
)

if %DYNAMIC_LIBS% (
  echo Building dynamic libraries - DLLs.
  set STATIC_LIBS=0==1
) else (
  echo Building static libraries where there's a choice of projects.
  echo    The rest of the project/solution files need to be set to static manually.
  set STATIC_LIBS=1==1
)

rem Use all CPU cores
set CL=/MP

rem Edit to suit your environment
SET MSVC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC
SET VCVERSION=140
SET BUILDTOOLS_PATH=E:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\
SET BUILDTOOLS_SCRIPT=vcvarsall.bat
rem If using the Visual C++ Build Tools, uncomment & adjust the following
rem SET BUILDTOOLS_PATH=C:\Program Files (x86)\Microsoft Visual C++ Build Tools
rem SET BUILDTOOLS_SCRIPT=vcbuildtools.bat
rem If building for XP and using the C++ Build Tools package (not full VS,)
rem   install the Windows 7.1 SDK then uncomment and adjust the following
rem SET WindowsSdkDir_71A=E:\Program Files\Microsoft SDKs\Windows\v7.1\

SET XCOPY=xcopy /S /Y /I
SET MSBUILD=msbuild /nologo /m /p:PlatformToolset=v%VCVERSION%_xp /p:RuntimeLibrary=%RUNTIMELIB%
rem If using the Visual C++ Build Tools, uncomment & adjust the following
rem SET MSBUILD=msbuild /m /p:PlatformToolset=v140_xp /p:VCTargetsPath="C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V140\\"
set ROOT_DIR=%CD%
SET BIN_DIR=%CD%\bin\
SET LIB_DIR=%CD%\lib\
SET INCLUDE_DIR=%CD%\include\
SET BUILD_DIR=%CD%\build\

IF "%4" == "nuke" (
  echo Deleting %LIB_DIR% and %INCLUDE_DIR%
  rd /S /Q %LIB_DIR%
  rd /S /Q %INCLUDE_DIR%
  rem Protoc.exe comes with the repo, so can't delete BIN_DIR
  rem   (Why don't we build Protoc? -Sean)
  rem rd /S /Q %BIN_DIR%
)

set OLDPATH=%PATH%
if %MACHINE_X86% (
  rem If build OS is 64-bit:
rem  call "%BUILDTOOLS_PATH%\vcbuildtools.bat" amd64_x86
call "%BUILDTOOLS_PATH%\%BUILDTOOLS_SCRIPT%" amd64_x86
  rem If build OS is 32-bit, uncomment the following:
  rem call "%BUILDTOOLS_PATH%\vcbuildtools.bat" x86
) else (
  rem If build OS is 64-bit:
rem  call "%BUILDTOOLS_PATH%\vcbuildtools.bat" amd64
call "%BUILDTOOLS_PATH%\%BUILDTOOLS_SCRIPT%" amd64
  rem If build OS is 32-bit, uncomment the following:
  rem call "%BUILDTOOLS_PATH%\vcbuildtools.bat" x86_amd64
)

md %LIB_DIR%
md %INCLUDE_DIR%
md %BIN_DIR%

rem Use our directories as well. Needed for a number of dependencies to find zlib, sqlite and xiph headers, including Qt.
set INCLUDE=%INCLUDE%;%INCLUDE_DIR%
set LIB=%LIB%;%LIB_DIR%
set UseEnv=true
rem Qt may need this
set LIBPATH=%LIBPATH%;%LIB_DIR%

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
call build_openssl.bat
call build_libshout.bat REM may soon depend on openssl
call build_libflac.bat
call build_libsndfile.bat
call build_fftw3.bat
call build_rubberband.bat REM depends on fftw3.h
call build_portaudio.bat
call build_hss1394.bat
call build_chromaprint.bat REM depends on fftw3
call build_taglib.bat REM depends on zlib 
REM We do not distribute LAME with Mixxx. If you wish to build it locally, uncomment.
REM call build_lame.bat
call build_qt4.bat REM depends on sqlite3, zlib and openssl

REM Copy debug runtime DLLs for debug builds.
if not %CONFIG_RELEASE% (
  %XCOPY% "%MSVC_PATH%\redist\Debug_NonRedist\%MACHINE_X%\Microsoft.VC%VCVERSION%.DebugCRT\*.dll" %LIB_DIR%
  %XCOPY% "%MSVC_PATH%\redist\Debug_NonRedist\%MACHINE_X%\Microsoft.VC%VCVERSION%.DebugCXXAMP\*.dll" %LIB_DIR%
  %XCOPY% "%MSVC_PATH%\redist\Debug_NonRedist\%MACHINE_X%\Microsoft.VC%VCVERSION%.DebugOpenMP\*.dll" %LIB_DIR%
)

REM Clean up after vcbuildtools.bat since repeated running eventually overflows PATH.
SET PATH=%OLDPATH%