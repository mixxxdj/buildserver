echo "Building taglib"
set TAGLIB_PATH=taglib-1.10

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

REM NOTE(pegasus): generated solution with
REM cmake . -G "Visual Studio 14 2015" -DCMAKE_SUPPRESS_REGENERATION=1 -DCMAKE_USE_RELATIVE_PATHS=1 -DZLIB_INCLUDE_DIR=..\..\include -DZLIB_LIBRARY=..\..\lib\zlibwapi.lib
 
cd build\%TAGLIB_PATH%
%MSBUILD% taglib.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:tag:Clean;tag:Rebuild

copy %PLATFORM%\%CONFIG%\tag.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\tag.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\tag.pdb %LIB_DIR%
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

cd %ROOT_DIR%