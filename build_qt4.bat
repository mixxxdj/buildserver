echo.
echo ---- Building Qt4 ----
SET QT4_PATH=qt-everywhere-opensource-src-4.8.7

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

REM Needed so stuff (like uic.exe) can find our zlibwapi.dll at run time
set PATH=%PATH%;%LIB_DIR%

REM NOTE(rryan): By setting -system-sqlite, -system-zlib is set as well.
REM NOTE(pegasus): To use the version we build, it's necessary to 
REM   replace "zdll" with "zlibwapi" in the following files:
REM   - src/3rdparty/zlib_dependency.pri
REM   - (maybe) src/tools/bootstrap/bootstrap.pri
REM   - (maybe) src/3rdparty/webkit/Source/WebKit2/win/WebKit2Common.vsprops
REM 
REM If you don't want to do that, just set -qt-zlib explicitly in the
REM   Configure options below.

REM patch Qt 4.8.7 to build on MSVC 2015.
REM (Get patch.exe from http://gnuwin32.sourceforge.net/packages/patch.htm)
..\patch.exe -p1 --verbose -i ..\qt-4.8.7-win.patch -r ..\qt4fix.rej.txt

echo Cleaning...
nmake /nologo distclean /K
nmake /nologo confclean /K

echo Building...
set CONFIG_ADD=
if NOT %STATIC_LIBS% ( SET CONFIG_ADD=-D ZLIB_WINAPI )
rem Might need -openssl-linked instead
configure.exe %CONFIG% -opensource -confirm-license -platform win32-msvc2015 -mp -system-sqlite -qt-sql-sqlite -system-zlib -ltcg -fast -static -D SQLITE_ENABLE_FTS3 -D SQLITE_ENABLE_FTS3_PARENTHESIS %CONFIG_ADD% -openssl -no-phonon -no-phonon-backend -no-multimedia -no-qt3support -no-dsp -no-vcproj -no-webkit -nomake demos -nomake examples -nomake tests
rem /K keeps building things not affected by errors
nmake /nologo

rem Remove obj files, supposedly
echo Cleaning up...
nmake /nologo clean /K

%XCOPY% bin\*.exe %BIN_DIR%
REM Don't copy DLLs or includes since we refer to them from QTDIR and the include files refer to the Qt source tree.
REM %XCOPY% lib\*.dll %LIB_DIR%
REM %XCOPY% lib\*.lib %LIB_DIR%
REM %XCOPY% include\* %INCLUDE_DIR%

cd %ROOT_DIR%