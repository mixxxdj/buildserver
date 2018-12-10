SETLOCAL
echo.
echo ---- Building Qt5 ----
REM NOTE(Be): manually clear the Jenkins workspace when updating Qt versions to avoid
REM wasting disk space for the old source code archive.
set QT_MAJOR=5
set QT_MINOR=12
set QT_PATCH=0

REM Qt5 paths can get really long. We use an abbreviated folder name to prevent hitting
REM Windows path length limits.
SET QTSHORTDIR=QT%QT_MAJOR%

REM remove old built files
rd /s /q build\%QTSHORTDIR%

set QTDIR=qt-everywhere-src-%QT_MAJOR%.%QT_MINOR%.%QT_PATCH%
if not exist %QTDIR%.tar.xz (
  echo --- Downloading Qt5 ---
  bitsadmin /transfer downloadQt5 /download http://download.qt.io/official_releases/qt/%QT_MAJOR%.%QT_MINOR%/%QT_MAJOR%.%QT_MINOR%.%QT_PATCH%/single/%QTDIR%.zip %CD%\%QTDIR%.zip
)

REM 7z requires separate extraction steps for the xz compression and the tar archive
7za x -obuild %QTDIR%.zip
move build\%QTDIR% build\%QTSHORTDIR%

SET VALRETURN=0

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=-release
) else (
  set CONFIG=-debug
)

cd build\%QTSHORTDIR%
IF ERRORLEVEL 1 (
echo could not find QT5 on %CD%\build\%QTSHORTDIR%
    SET VALRETURN=1
	goto END
)

REM Apply workaround for QTBUG-61342.
%BIN_DIR%\patch.exe -N -p0 --verbose -i %CD%\..\QTBUG-61342.patch -r %CD%\..\QTBUG-61342.rej.txt
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

REM nmake distclean or nmake confclean are not present in the Makefile, so we delete these files and hope it rebuilds.
del qtbase\.qmake.cache
del qtbase\config.log
del /S /Q qtbase\mkspecs\modules\*.pri
del /S /Q qtbase\mkspecs\modules-inst\*.pri

echo Building...

set QT_NOMAKE=-nomake examples -nomake tests
REM skips can be any directory starting with 'qt' at the root of the repo -- keep list alphabetized.
REM skipping qttools skips building translations (since it doesn't have lrelease)
set QT_SKIP=-skip qt3d -skip qtdoc -skip qtmultimedia -skip qtwebengine -skip qtwebview -skip qtconnectivity -skip qtpurchasing -skip qtgamepad -skip qtlocation -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech

REM We link against the system SQLite so that Mixxx can link with and use the
REM same instance of the SQLite library in our binary (for example, so we
REM can install custom functions).
REM -D NOMINMAX https://forum.qt.io/topic/21605/solved-qt5-vs2010-qdatetime-not-enough-actual-parameters-for-macro-min-max
set QT_COMMON=-prefix %ROOT_DIR%\Qt-%QT_MAJOR%.%QT_MINOR%.%QT_PATCH%\ -opensource -confirm-license -platform win32-msvc2017 -force-debug-info -no-strip -mp -system-sqlite -sql-sqlite -no-sql-odbc -system-zlib -ltcg -D NOMINMAX -D SQLITE_ENABLE_FTS3 -D SQLITE_ENABLE_FTS3_PARENTHESIS -D ZLIB_WINAPI %QT_NOMAKE% %QT_SKIP% -no-dbus -opengl dynamic -qt-pcre -qt-libpng -qt-harfbuzz

if %STATIC_LIBS% (
call configure.bat %CONFIG% %QT_COMMON% -static -openssl-linked OPENSSL_LIBS="-luser32 -ladvapi32 -lgdi32 -lcrypt32 -lssleay32 -llibeay32"
) else (
call configure.bat %CONFIG% %QT_COMMON% -shared -openssl -separate-debug-info
)

IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

rem /K keeps building things not affected by errors
nmake /nologo
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

rem Now install to %ROOT_DIR%.
nmake install
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

rem Note, we do not run nmake clean because it deletes files we need (e.g. compiled translations).

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%
