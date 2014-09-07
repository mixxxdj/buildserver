echo "Building taglib"
set TAGLIB_PATH=taglib-1.9.1

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

REM NOTE(rryan): generated solution with
REM cmake . -G "Visual Studio 12 2013" -DZLIB_INCLUDE_DIR=..\..\include -DZLIB_LIBRARY=..\..\lib\zlibwapi.lib
 
cd build\%TAGLIB_PATH%
%MSBUILD% taglib.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:tag:Clean;tag:Rebuild

copy %PLATFORM%\%CONFIG%\tag.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\tag.dll %LIB_DIR%
md %INCLUDE_DIR%\taglib
xcopy taglib\ape\*.h %INCLUDE_DIR%\taglib
xcopy taglib\asf\*.h %INCLUDE_DIR%\taglib
xcopy taglib\flac\*.h %INCLUDE_DIR%\taglib
xcopy taglib\it\*.h %INCLUDE_DIR%\taglib
xcopy taglib\mod\*.h %INCLUDE_DIR%\taglib
xcopy taglib\mp4\*.h %INCLUDE_DIR%\taglib
xcopy taglib\mpc\*.h %INCLUDE_DIR%\taglib
xcopy taglib\mpeg\*.h %INCLUDE_DIR%\taglib
xcopy taglib\mpeg\id3v1\*.h %INCLUDE_DIR%\taglib
xcopy taglib\mpeg\id3v2\*.h %INCLUDE_DIR%\taglib
xcopy taglib\mpeg\id3v2\frames\*.h %INCLUDE_DIR%\taglib
xcopy taglib\ogg\*.h %INCLUDE_DIR%\taglib
xcopy taglib\ogg\flac\*.h %INCLUDE_DIR%\taglib
xcopy taglib\ogg\opus\*.h %INCLUDE_DIR%\taglib
xcopy taglib\ogg\speex\*.h %INCLUDE_DIR%\taglib
xcopy taglib\ogg\vorbis\*.h %INCLUDE_DIR%\taglib
xcopy taglib\riff\*.h %INCLUDE_DIR%\taglib
xcopy taglib\riff\aiff\*.h %INCLUDE_DIR%\taglib
xcopy taglib\riff\wav\*.h %INCLUDE_DIR%\taglib
xcopy taglib\s3m\*.h %INCLUDE_DIR%\taglib
xcopy taglib\toolkit\*.h %INCLUDE_DIR%\taglib
xcopy taglib\trueaudio\*.h %INCLUDE_DIR%\taglib
xcopy taglib\wavpack\*.h %INCLUDE_DIR%\taglib
xcopy taglib\xm\*.h %INCLUDE_DIR%\taglib

cd %ROOT_DIR%