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
configure.exe %CONFIG% -opensource -confirm-license -platform win32-msvc2013 -qt-sql-sqlite -ltcg -fast -shared -D SQLITE_ENABLE_FTS3 -D SQLITE_ENABLE_FTS3_PARENTHESIS -no-phonon -no-multimedia -no-qt3support -no-dsp -no-vcproj -nomake demos -nomake examples -nomake tests
nmake

%XCOPY% bin\*.exe %BIN_DIR%
REM Don't copy DLLs or includes since we refer to them from QTDIR and the include files refer to the Qt source tree.
REM %XCOPY% lib\*.dll %LIB_DIR%
REM %XCOPY% lib\*.lib %LIB_DIR%
REM %XCOPY% include\* %INCLUDE_DIR%

cd %ROOT_DIR%