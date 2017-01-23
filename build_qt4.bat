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

REM Needed so stuff (like uic.exe) can find our zlibwapi.dll at run time
set PATH=%PATH%;%LIB_DIR%

REM patch Qt 4.8.7 to build on MSVC 2015.
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
REM We link against the system SQLite so that Mixxx can link with and use the 
REM same instance of the SQLite library in our binary (for example, so we 
REM can install custom functions).
REM NOTE(rryan): By setting -system-sqlite, -system-zlib is set as well.
set QT_COMMON=-opensource -confirm-license -platform win32-msvc2015 -mp -system-sqlite -qt-sql-sqlite -system-zlib -ltcg -fast -D _USING_V110_SDK71_ -D SQLITE_ENABLE_FTS3 -D SQLITE_ENABLE_FTS3_PARENTHESIS -D ZLIB_WINAPI -no-phonon -no-phonon-backend -no-multimedia -no-qt3support -no-dsp -no-vcproj -no-webkit -nomake demos -nomake examples -nomake tests

if %STATIC_LIBS% (
configure.exe %CONFIG% %QT_COMMON% -static -openssl-linked OPENSSL_LIBS="-luser32 -ladvapi32 -lgdi32 -lcrypt32 -lssleay32 -llibeay32"
) else (
configure.exe %CONFIG% %QT_COMMON% -shared -openssl
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

rem Note, we do not run nmake clean because it deletes files we need (e.g. compiled translations).

REM Don't copy binaries or DLLs or includes since we refer to them from QTDIR and the include files refer to the Qt source tree.
REM %XCOPY% bin\*.exe %BIN_DIR%
REM %XCOPY% lib\*.dll %LIB_DIR%
REM %XCOPY% lib\*.lib %LIB_DIR%
REM %XCOPY% include\* %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%