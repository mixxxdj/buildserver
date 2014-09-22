echo "Building taglib"
set TAGLIB_PATH=taglib-1.9.1

if %MACHINE_X86% (
  set PLATFORM=Win32
  set CMAKE_CONF="Visual Studio 12 2013"
) else (
  set PLATFORM=x64
  set CMAKE_CONF="Visual Studio 12 2013 Win64"
)

if %CONFIG_RELEASE% (
  set CONFIG=Release
) else (
  set CONFIG=Debug
)

cd build\%TAGLIB_PATH%
cmake . -G %CMAKE_CONF% -DZLIB_INCLUDE_DIR=%INCLUDE_DIR% -DZLIB_LIBRARY=%LIB_DIR%\zlibwapi.lib
%MSBUILD% taglib.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:tag:Clean;tag:Rebuild

copy taglib\%CONFIG%\tag.lib %LIB_DIR%
copy taglib\%CONFIG%\tag.dll %LIB_DIR%
md %INCLUDE_DIR%\taglib
xcopy /y taglib_config.h %INCLUDE_DIR%\taglib
xcopy /y taglib\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\ape\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\asf\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\flac\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\it\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\mod\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\mp4\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\mpc\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\mpeg\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\mpeg\id3v1\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\mpeg\id3v2\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\mpeg\id3v2\frames\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\ogg\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\ogg\flac\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\ogg\opus\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\ogg\speex\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\ogg\vorbis\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\riff\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\riff\aiff\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\riff\wav\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\s3m\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\toolkit\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\toolkit\*.tcc %INCLUDE_DIR%\taglib
xcopy /y taglib\trueaudio\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\wavpack\*.h %INCLUDE_DIR%\taglib
xcopy /y taglib\xm\*.h %INCLUDE_DIR%\taglib

cd %ROOT_DIR%