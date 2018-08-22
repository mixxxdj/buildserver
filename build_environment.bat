@echo off
SETLOCAL

REM ==================================
REM Path setup and  initial checks
REM ==================================

IF "%ProgramW6432%" =="" (
SET OS_IS_64BIT=0==1
SET PROGRAMFILES_PATH=%ProgramFiles%
) else (
SET OS_IS_64BIT=1==1
REM doublequote the whole SET command prevents an error with parentheses
SET "PROGRAMFILES_PATH=%ProgramFiles(x86)%"
)


rem ====== Edit to suit your environment =========
SET VCVERSION=140

REM Allow overriding MSSDKS_PATH from outside this script.
IF "%MSSDKS_PATH%" == "" (
  set "MSSDKS_PATH=%PROGRAMFILES_PATH%\Microsoft SDKs"
)

REM Allow overriding MSVC_PATH from outside this script.
IF "%MSVC_PATH%" == "" (
  SET "MSVC_PATH=%PROGRAMFILES_PATH%\Microsoft Visual Studio 14.0"
)

REM Allow overriding BUILDTOOLS_PATH from outside this script.
if "%BUILDTOOLS_PATH%" == "" (
  SET "BUILDTOOLS_PATH=%PROGRAMFILES_PATH%\Microsoft Visual C++ Build Tools"
)

REM Allow overriding CMAKEDIR from outside this script.
if "%CMAKEDIR%" == "" (
  SET "CMAKEDIR=%PROGRAMFILES_PATH%\CMake\bin"
)

REM with two slashes at the end
SET "PLAT_TARGETS_PATH=%PROGRAMFILES_PATH%\MSBuild\Microsoft.Cpp\v4.0\V140\\"
SET "NASMPATH=%CD%\bin"

REM Verify paths.
IF EXIST "%MSVC_PATH%" (
SET "BUILDTOOLS_PATH=%MSVC_PATH%\VC"
SET BUILDTOOLS_SCRIPT=vcvarsall.bat
SET MSBUILD=msbuild /nologo /m /p:PlatformToolset=v%VCVERSION%_xp /p:RuntimeLibrary=%RUNTIMELIB%

REM Check whether we have a 64-bit compiler available.
REM NOTE(rryan): Temporarily disabled because the build doesn't work with a 64-bit compiler.
rem IF EXIST "%MSVC_PATH%\VC\bin\amd64\cl.exe" (
rem SET COMPILER_X86=amd64_x86
rem SET COMPILER_X64=amd64
rem ) ELSE (
SET COMPILER_X86=x86
SET COMPILER_X64=x86_amd64
rem )

) ELSE (
IF EXIST "%BUILDTOOLS_PATH%" (
SET BUILDTOOLS_SCRIPT=vcbuildtools.bat
SET MSBUILD=msbuild /m /p:PlatformToolset=v140_xp /p:VCTargetsPath="%PLAT_TARGETS_PATH%"
rem If building for XP and using the C++ Build Tools package (not full VS,)
rem   install the Windows 7.1 SDK and adjust the following if needed
SET "WindowsSdkDir_71A=%MSSDKS_PATH%\Windows\v7.1A\"

SET COMPILER_X86=amd64_x86
SET COMPILER_X64=amd64
) ELSE (
echo.
echo Could not find "%MSVC_PATH%" nor "%BUILDTOOLS_PATH%".
echo Edit the build_environment.bat file and/or install the required software
echo http://landinghub.visualstudio.com/visual-cpp-build-tools
echo https://www.microsoft.com/en-us/download/details.aspx?id=8279
exit /b 1
)
REM END EXIST BUILDTOOLS
)
REM END EXIST VISUALSTUDIO

if NOT EXIST "%CMAKEDIR%" (
  set "CMAKEDIROLD=%CMAKEDIR%"
  set "CMAKEDIR=%ProgramFiles%\CMake\bin"
)
if NOT EXIST "%CMAKEDIR%" (
echo.
echo could not find CMake path on "%CMAKEDIROLD%" or "%CMAKEDIR%"
echo edit the build_environment.bat file and/or install the required software
echo https://cmake.org/
exit /b 1
)
if NOT EXIST "%NASMPATH%" (
echo.
echo Could not find NASM path on %NASMPATH%, needed for libFLAC and others
echo Edit the build_environment.bat file and/or install the required software
echo http://www.nasm.us/
exit /b 1
)

if NOT EXIST "%CD%\build\pa_stable_v190600_20161030\src\hostapi\asio\ASIOSDK" (
echo.
echo You need to obtain and copy the ASIO SDK to the folder:
echo %CD%\build\pa_stable_v190600_20161030\src\hostapi\asio\ASIOSDK
echo http://www.steinberg.net/en/company/developer.html
exit /b 1
)

python -V > NUL
IF ERRORLEVEL 1 (
echo.
echo Python is not in the PATH. Install it or set the path correctly
echo https://www.python.org/
exit /b 1
)

perl -v > NUL
IF ERRORLEVEL 1 (
echo.
echo Perl is not in the PATH. Install it or set the path correctly
echo https://www.perl.org
exit /b 1
)


REM ==================================
REM Parameter reading and variable setup
REM ==================================
REM ^ is the escape character.
if "%2" == "" (
  echo Missing parameters. Usage: ^(Note: keep same case and order^)
  echo.
  echo build_environment.bat x64^|x86 Debug^|Release^|ReleaseFastbuild [Dynamic][nuke^|jumpIdx]
  echo.
  echo Example: build_environment.bat x64 Release Dynamic nuke
  echo.
  echo It is possible to jump to a task indicating the number:
  echo build_environment.cmd x64 Release 7
  echo.
  echo To build only one and stop add an asterisk:
  echo build_environment.cmd x64 Release *7
  exit /b 1
)

set JUMPIDX=
set MACHINE_X86="%1" == "x86"

if "%2" == "Release" (
  set CONFIG_RELEASE=1==1
  set CONFIG_FASTBUILD=0==1
) else (
  if "%2" == "ReleaseFastbuild" (
    set CONFIG_RELEASE=1==1
    set CONFIG_FASTBUILD=1==1
  ) else (
    set CONFIG_RELEASE=0==1
    set CONFIG_FASTBUILD=0==1
  )
)

if "%3" == "Dynamic" (
set DYNAMIC_LIBS="%3" == "Dynamic"
if "%4" == "nuke" (
set DELETE_OLD="%4" == "nuke"
) else ( REM not nuke
set DELETE_OLD="" == "nuke"
if NOT "%4" == "" (
set JUMPIDX=%4
)
)
REM END ELSE nuke
) else ( REM not Dynamic
set DYNAMIC_LIBS="" == "Dynamic"
if "%3" == "nuke" (
set DELETE_OLD="%3" == "nuke"
) else ( REM not nuke
set DELETE_OLD="" == "nuke"
if NOT "%3" == "" (
set JUMPIDX=%3
)
)
REM END ELSE nuke
)
REM END ELSE Dynamic

if %MACHINE_X86% (
  echo Building for x86.
  set MACHINE_X=x86
  set FOLDER_PLATFORM=Win32
) else (
  echo Building for x64.
  set MACHINE_X=x64
  set FOLDER_PLATFORM=x64
)

if %CONFIG_RELEASE% (
  echo Building release mode.
  set RUNTIMELIB="MultiThreadedDLL"
  REM the above doesn't actually seem to work, so we do the following
  set RUNTIME_FLAG=/MD
) else (
  echo Building debug mode.
  set RUNTIMELIB="MultiThreadedDebugDLL"
  set RUNTIME_FLAG=/MDd
)
SET BUILDTYPEDIR=%2
if %DYNAMIC_LIBS% (
  echo Building dynamic libraries - DLLs.
  set STATIC_LIBS=0==1
) else (
  SET BUILDTYPEDIR=%2Static
  echo Building static libraries where there's a choice of projects.
  echo    The rest of the project/solution files need to be set to static manually.
  set STATIC_LIBS=1==1
)

rem /MP Use all CPU cores.
rem /EHsc Do not handle SEH in try / except blocks.
rem /FS force synchronous PDB writes (prevents PDB corruption with /MP)
rem /Zi write PDBs
rem /Zc:threadSafeInit- disable C++11 magic static support (Bug #1653368)
rem /DEBUG moves debug information from the .o files to the .pdb during link.
set XP_SUPPORT=/D _USING_V110_SDK71_
set _CL_=/MP /FS /EHsc /Zi /Zc:threadSafeInit- %RUNTIME_FLAG% %XP_SUPPORT%
set _LINK_=/DEBUG

if %CONFIG_RELEASE% (
  if %CONFIG_FASTBUILD% (
    REM In fastbuild mode, disable /GL (whole program optimization) and /LTCG (link-time code generation).
    set _CL_=%_CL_% /GL-
    REM /DEBUG defaults /OPT:REF and /OPT:ICF to off, so enable them explicitly.
    set _LINK_=%_LINK_% /LTCG:OFF /OPT:REF /OPT:ICF
  ) else (
    REM In release mode, use /GL and /LTCG.
    REM TODO(rryan): Try enabling /Gy here?
    REM /DEBUG defaults /OPT:REF and /OPT:ICF to off, so enable them explicitly.
    set _CL_=%_CL_% /GL
    set _LINK_=%_LINK_% /LTCG:NOSTATUS /OPT:REF /OPT:ICF
  )
) else (
  REM Use project defaults in debug mode.
)

SET XCOPY=xcopy /S /Y /I

set OLDPATH=%PATH%
set ROOT_DIR=%CD%
SET BIN_DIR=%ROOT_DIR%\bin\
SET LIB_DIR=%ROOT_DIR%\lib\
SET INCLUDE_DIR=%ROOT_DIR%\include\
SET BUILD_DIR=%CD%\build\

REM Everyting prepared. Setup the compiler.
if %MACHINE_X86% (
  call "%BUILDTOOLS_PATH%\%BUILDTOOLS_SCRIPT%" %COMPILER_X86%
) else (
  call "%BUILDTOOLS_PATH%\%BUILDTOOLS_SCRIPT%" %COMPILER_X64%
)

REM The Visual C++ compiler (cl.exe) recognizes certain environment variables, specifically LIB, LIBPATH, PATH, and INCLUDE
rem Use our directories as well. Needed for a number of dependencies to find zlib, sqlite and xiph headers, including Qt.
set INCLUDE=%INCLUDE%;%INCLUDE_DIR%
set LIB=%LIB%;%LIB_DIR%
set UseEnv=true
rem Qt may need this
set LIBPATH=%LIBPATH%;%LIB_DIR%


REM ==================================
REM Deletion and directory preparation
REM ==================================

IF %DELETE_OLD% (
  echo Deleting %LIB_DIR% and %INCLUDE_DIR%
  rd /S /Q %LIB_DIR%
  rd /S /Q %INCLUDE_DIR%
  rd /S /Q %BIN_DIR%
)
md %LIB_DIR%
md %INCLUDE_DIR%
md %BIN_DIR%
REM restore checked-in binaries
git checkout bin\*

echo Beginning build with environment:
set

REM ==================================
REM Script execution to compile each lib
REM ==================================

SETLOCAL EnableDelayedExpansion
SET PROCESSALL=1==1
IF [%JUMPIDX%] == [] (
  SET JUMPIDX=1
) ELSE (
  SET firstchar=%JUMPIDX:~0,1%
  IF !firstchar! == * (
    SET PROCESSALL=0==1
    SET JUMPIDX=%JUMPIDX:~1%
  )
)
SET /A TASKNUM=1

 REM build_libid3tag.bat depends on zlib
 REM build_libopus.bat depends on libogg
 REM build_libshout.bat depends on openssl
 REM build_libsndfile.bat depends on flac
 REM build_rubberband.bat depends on fftw3
 REM build_chromaprint.bat depends on fftw3
 REM build_taglib.bat depends on zlib
 REM build_qt4.bat depends on sqlite3, zlib, openssl
 REM build_qtkeychain.bat depends on qt5
FOR %%G IN (
build_lame.bat,
build_sqlite3.bat,
build_zlib.bat,
build_pthreads.bat,
build_protobuf.bat,
build_portmidi.bat,
build_libid3tag.bat,
build_libmad.bat,
build_libogg.bat,
build_libopus.bat,
build_libvorbis.bat,
build_openssl.bat,
build_libshout.bat,
build_libflac.bat,
build_libsndfile.bat,
build_fftw3.bat,
build_rubberband.bat,
build_portaudio.bat,
build_hss1394.bat,
build_chromaprint.bat,
build_taglib.bat,
build_qt5.bat,
build_qtkeychain.bat) DO (
  IF !TASKNUM! EQU %JUMPIDX% (
	SET EXECUTE=1
  ) ELSE (
    SET EXECUTE=0
	IF !TASKNUM! GTR %JUMPIDX% (
	  IF %PROCESSALL% (
	    SET EXECUTE=1
	  )
    )
  )
  IF !EXECUTE! EQU 1 (
    call %%G
    IF ERRORLEVEL 1 (
	  echo ==============================
	  echo.
      echo Task !TASKNUM! Failed
      ENDLOCAL
	  ENDLOCAL
      exit /b 1
	)
  )
  SET /A TASKNUM=!TASKNUM!+1
)
ENDLOCAL


REM Copy debug runtime DLLs for debug builds.
if not %CONFIG_RELEASE% (
  %XCOPY% "%MSVC_PATH%\VC\redist\Debug_NonRedist\%MACHINE_X%\Microsoft.VC%VCVERSION%.DebugCRT\*.dll" %LIB_DIR%
  %XCOPY% "%MSVC_PATH%\VC\redist\Debug_NonRedist\%MACHINE_X%\Microsoft.VC%VCVERSION%.DebugCXXAMP\*.dll" %LIB_DIR%
  %XCOPY% "%MSVC_PATH%\VC\redist\Debug_NonRedist\%MACHINE_X%\Microsoft.VC%VCVERSION%.DebugOpenMP\*.dll" %LIB_DIR%
)
echo.
echo.
echo Everything was built successfully
echo.

ENDLOCAL
