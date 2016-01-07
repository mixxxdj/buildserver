echo "Building Qt5"
REM SET QT5_PATH=qt-everywhere-opensource-src-5.5.1
REM Qt5 has long paths so we run into the NTFS file path limit. Trim the directory name as much as possible to save on characters. (Yes, really.)
SET QT5_PATH=qt551

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

cd build\%QT5_PATH%

REM We link against the system SQLite so that Mixxx can link with and use the 
REM same instance of the SQLite library in our binary (for example, so we 
REM can install custom functions).

REM Add %INCLUDE_DIR% and %LIB_DIR% to paths so Qt can find our version of sqlite3.
set INCLUDE=%INCLUDE%;%INCLUDE_DIR%
set LIB=%LIB%;%LIB_DIR%
set LIBPATH=%LIBPATH%;%LIB_DIR%

set PATH=%CD%\qtbase\bin;%CD%\gnuwin32\bin;%PATH%
set QMAKESPEC=win32-msvc2013

REM NOTE(rryan): By setting -system-sqlite, -system-zlib is set as well. Building with -system-zlib fails with missing "zlib.dll". So I think our zlib build is not currently ready to be used with Qt. We should fix this but for now use -qt-zlib.
REM NOTE(rryan): For some reason configure.bat returns an error even if it succeeded. Use "& nmake" to build regardless of the result.
configure %CONFIG% -opensource -confirm-license -platform %QMAKESPEC% -qt-zlib -system-sqlite -qt-sql-sqlite -c++11 -ltcg -shared -D SQLITE_ENABLE_FTS3 -D SQLITE_ENABLE_FTS3_PARENTHESIS -skip multimedia -skip qt3d -skip qtwebkit -skip qtwebkit-examples -skip qtwebengine -nomake examples -nomake tests -no-dbus -no-audio-backend & nmake

REM Copy uic, rcc, and other utilities.
%XCOPY% qtbase\bin\*.exe %BIN_DIR%
REM Don't copy DLLs or includes since we refer to them from QTDIR and the include files refer to the Qt source tree.

cd %ROOT_DIR%