SETLOCAL
echo "Building taglib"
set TAGLIB_PATH=taglib-1.11.1
SET VALRETURN=0

if %MACHINE_X86% (
  set PLATFORM=Win32
  set COMPILERPROJECT="Visual Studio 14 2015"
) else (
  set PLATFORM=x64
  set COMPILERPROJECT="Visual Studio 14 2015 Win64"
)

if %CONFIG_RELEASE% (
  set CONFIG=Release
) else (
  set CONFIG=Debug
)

set ZLIBNAME=zlibwapi.lib

if %STATIC_LIBS% (
    set DYNAMIC=OFF
) else (
    set DYNAMIC=ON
)

cd build\%TAGLIB_PATH%
REM Re-generate solution files.
rd /S /Q %PLATFORM%
mkdir %PLATFORM%
cd %PLATFORM%
REM NOTE(pegasus): generated solution with

"%CMAKEDIR%\cmake" ../ -G %COMPILERPROJECT% -DBUILD_SHARED_LIBS=%DYNAMIC% -DCMAKE_SUPPRESS_REGENERATION=1 -DCMAKE_USE_RELATIVE_PATHS=1 -DZLIB_INCLUDE_DIR=%INCLUDE_DIR% -DZLIB_LIBRARY=%LIB_DIR%\%ZLIBNAME%
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)
 
%MSBUILD% taglib.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:tag:Clean;tag:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy taglib\%CONFIG%\tag.lib %LIB_DIR%
if NOT %STATIC_LIBS% ( copy taglib\%CONFIG%\tag.dll %LIB_DIR% )
copy taglib\%CONFIG%\tag.pdb %LIB_DIR%
cd ..
md %INCLUDE_DIR%\taglib
%XCOPY% taglib_config.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\ape\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\asf\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\flac\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\it\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\mod\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\mp4\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\mpc\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\mpeg\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\mpeg\id3v1\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\mpeg\id3v2\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\mpeg\id3v2\frames\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\ogg\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\ogg\flac\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\ogg\opus\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\ogg\speex\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\ogg\vorbis\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\riff\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\riff\aiff\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\riff\wav\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\s3m\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\toolkit\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\toolkit\*.tcc %INCLUDE_DIR%\taglib
%XCOPY% taglib\trueaudio\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\wavpack\*.h %INCLUDE_DIR%\taglib
%XCOPY% taglib\xm\*.h %INCLUDE_DIR%\taglib

SET VALRETURN=1
goto END

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%
