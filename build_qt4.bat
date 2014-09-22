echo "Building Qt4"
SET QT4_PATH=qt-everywhere-opensource-src-4.8.6

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

cd build\%QT4_PATH%

REM We link against the system SQLite so that Mixxx can link with and use the 
REM same instance of the SQLite library in our binary (for example, so we 
REM can install custom functions).

REM Add %INCLUDE_DIR% and %LIB_DIR% to paths so Qt can find our version of sqlite3.
set INCLUDE=%INCLUDE%;%INCLUDE_DIR%
set LIB=%LIB%;%LIB_DIR%
set LIBPATH=%LIBPATH%;%LIB_DIR%

REM NOTE(rryan): By setting -system-sqlite, -system-zlib is set as well. Set -qt-zlip explicitly.
REM At some point in the future we could look at using external zlib (since we build it already)
REM but I don't want to right now.
configure.exe %CONFIG% -opensource -confirm-license -platform win32-msvc2013 -qt-zlib -system-sqlite -qt-sql-sqlite -ltcg -fast -shared -D SQLITE_ENABLE_FTS3 -D SQLITE_ENABLE_FTS3_PARENTHESIS -no-phonon -no-multimedia -no-qt3support -no-dsp -no-vcproj -nomake demos -nomake examples -nomake tests
nmake

%XCOPY% bin\*.exe %BIN_DIR%
REM Don't copy DLLs or includes since we refer to them from QTDIR and the include files refer to the Qt source tree.
REM %XCOPY% lib\*.dll %LIB_DIR%
REM %XCOPY% lib\*.lib %LIB_DIR%
REM %XCOPY% include\* %INCLUDE_DIR%

cd %ROOT_DIR%