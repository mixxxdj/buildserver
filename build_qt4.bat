SETLOCAL
echo.
echo ---- Building Qt4 ----
SET QT4_PATH=qt-everywhere-opensource-src-4.8.7
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

cd build\%QT4_PATH%
IF ERRORLEVEL 1 (
echo could not find QT4 on %CD%\build\%QT4_PATH%
    SET VALRETURN=1
	goto END
)

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
REM No need. I've copied the msys2 version to the bin folder.
%BIN_DIR%\patch.exe -N -p1 --verbose -i %CD%\..\qt-4.8.7-win.patch -r %CD%\..\qt4fix.rej.txt
REM ERRORLEVEL 2 since it will report errorlevel 1 when already applied
IF ERRORLEVEL 2 (
    SET VALRETURN=1
	goto END
)

REM nmake distclean or nmake confclean are not present in the Makefile, so we delete these files and hope it rebuilds.
del .qmake.cache
del config.log
del /S /Q mkspecs/modules/*.pri \
del /S /Q mkspecs/modules-inst/*.pri


echo Building...
set CONFIG_ADD=
if %STATIC_LIBS% (
rem Might need -openssl-linked instead
configure.exe %CONFIG% -opensource -confirm-license -platform win32-msvc2015 -mp -system-sqlite -qt-sql-sqlite -system-zlib -ltcg -fast -s -D SQLITE_ENABLE_FTS3 -D SQLITE_ENABLE_FTS3_PARENTHESIS -openssl -no-phonon -no-phonon-backend -no-multimedia -no-qt3support -no-dsp -no-vcproj -no-webkit -nomake demos -nomake examples -nomake tests
) else (
configure.exe %CONFIG% -opensource -confirm-license -platform win32-msvc2015 -mp -system-sqlite -qt-sql-sqlite -system-zlib -ltcg -fast -shared -D SQLITE_ENABLE_FTS3 -D SQLITE_ENABLE_FTS3_PARENTHESIS -D ZLIB_WINAPI -openssl -no-phonon -no-phonon-backend -no-multimedia -no-qt3support -no-dsp -no-vcproj -no-webkit -nomake demos -nomake examples -nomake tests
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

rem Remove obj files, supposedly
echo Cleaning up...
nmake /nologo clean /K
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

%XCOPY% bin\*.exe %BIN_DIR%
REM Don't copy DLLs or includes since we refer to them from QTDIR and the include files refer to the Qt source tree.
REM %XCOPY% lib\*.dll %LIB_DIR%
REM %XCOPY% lib\*.lib %LIB_DIR%
REM %XCOPY% include\* %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%