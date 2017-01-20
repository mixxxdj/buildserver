REM Exports a minimal environment from a build environment.
@echo off
SETLOCAL
set COPY=copy /Y
set XCOPY=xcopy /Y /I /E /Q

if "%1" == "" (
  echo Usage: export_minimal_environment.bat winlib_dir [minimal_destination]
  echo.
  echo If minimal_destination is not provided, "winlib_dir-minimal" is used.
  ENDLOCAL
  exit /b 1
)

set "ENVIRONMENT_PATH=%1"

if "%2" == "" (
set "MINIMAL_PATH=%ENVIRONMENT_PATH%-minimal"
) else (
set "MINIMAL_PATH=%2"
)

md "%MINIMAL_PATH%"
IF ERRORLEVEL 1 (
  echo Error creating "%MINIMAL_PATH%".
  ENDLOCAL
  exit /b 1
)

REM copy the build log if it exists -- crucial for build archaeology
if exist "%ENVIRONMENT_PATH%\build_environment.log" (
  %COPY% "%ENVIRONMENT_PATH%\build_environment.log" "%MINIMAL_PATH%\"
)

%COPY% "%ENVIRONMENT_PATH%\vc_redist.x86.exe" "%MINIMAL_PATH%\"
%COPY% "%ENVIRONMENT_PATH%\vc_redist.x64.exe" "%MINIMAL_PATH%\"
%XCOPY% "%ENVIRONMENT_PATH%\bin" "%MINIMAL_PATH%\bin"
%XCOPY% "%ENVIRONMENT_PATH%\lib" "%MINIMAL_PATH%\lib"
%XCOPY% "%ENVIRONMENT_PATH%\include" "%MINIMAL_PATH%\include"

set QT_VERSION=4.8.7
set "QT_DIR=%ENVIRONMENT_PATH%\build\qt-everywhere-opensource-src-%QT_VERSION%"
set "QT_MINIMAL_DIR=%MINIMAL_PATH%\Qt-%QT_VERSION%\"

md "%QT_MINIMAL_DIR%"

REM only copy binaries we need
md "%QT_MINIMAL_DIR%\bin"
%COPY% "%QT_DIR%\bin\lconvert.exe"  "%QT_MINIMAL_DIR%\bin"
%COPY% "%QT_DIR%\bin\lrelease.exe"  "%QT_MINIMAL_DIR%\bin"
%COPY% "%QT_DIR%\bin\lupdate.exe"  "%QT_MINIMAL_DIR%\bin"
%COPY% "%QT_DIR%\bin\moc.exe"  "%QT_MINIMAL_DIR%\bin"
%COPY% "%QT_DIR%\bin\rcc.exe"  "%QT_MINIMAL_DIR%\bin"
%COPY% "%QT_DIR%\bin\uic.exe"  "%QT_MINIMAL_DIR%\bin"

%XCOPY% "%QT_DIR%\include" "%QT_MINIMAL_DIR%\include"

REM manually copy only libs Mixxx uses
md "%QT_MINIMAL_DIR%\lib"
%COPY% "%QT_DIR%\lib\qtmain.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtCore.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtGui.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtNetwork.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtOpenGL.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtScript.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtScriptTools.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtSql.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtSvg.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtTest.lib"  "%QT_MINIMAL_DIR%\lib\"
%COPY% "%QT_DIR%\lib\QtXml.lib"  "%QT_MINIMAL_DIR%\lib\"


%XCOPY% "%QT_DIR%\plugins" "%QT_MINIMAL_DIR%\plugins"

REM only copy header files in src.
%XCOPY% "%QT_DIR%\src\*.h" "%QT_MINIMAL_DIR%\src"

REM manually copy PDBs
REM TODO(rryan) Qt5 offers a "release with debug info" option.
%COPY% "%QT_DIR%\src\corelib\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtCore.pdb"
REM skipping declarative -- add it back in for Qt5.
%COPY% "%QT_DIR%\src\gui\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtGui.pdb"
%COPY% "%QT_DIR%\src\network\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtNetwork.pdb"
%COPY% "%QT_DIR%\src\opengl\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtOpenGL.pdb"
%COPY% "%QT_DIR%\src\plugins\accessible\widgets\vc140.pdb"  "%QT_MINIMAL_DIR%\plugins\accessible\qtaccessiblewidgets.pdb"
%COPY% "%QT_DIR%\src\plugins\iconengines\svgiconengine\vc140.pdb"  "%QT_MINIMAL_DIR%\plugins\iconengines\qsvgicon.pdb"
%COPY% "%QT_DIR%\src\plugins\imageformats\ico\vc140.pdb"  "%QT_MINIMAL_DIR%\plugins\imageformats\qico.pdb"
%COPY% "%QT_DIR%\src\plugins\imageformats\svg\vc140.pdb"  "%QT_MINIMAL_DIR%\plugins\imageformats\qsvg.pdb"
%COPY% "%QT_DIR%\src\plugins\imageformats\tga\vc140.pdb"  "%QT_MINIMAL_DIR%\plugins\imageformats\qtga.pdb"
%COPY% "%QT_DIR%\src\script\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtScript.pdb"
%COPY% "%QT_DIR%\src\scripttools\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtScriptTools.pdb"
%COPY% "%QT_DIR%\src\sql\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtSql.pdb"
%COPY% "%QT_DIR%\src\svg\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtSvg.pdb"
%COPY% "%QT_DIR%\src\testlib\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtTest.pdb"
%COPY% "%QT_DIR%\src\xml\vc140.pdb"  "%QT_MINIMAL_DIR%\lib\QtXml.pdb"

REM only copy compiled qt_ translations.
%XCOPY% "%QT_DIR%\translations\qt_*.qm" "%QT_MINIMAL_DIR%\translations"
REM The above also copies qt_help_*, which we don't need.
del "%QT_MINIMAL_DIR%\translations\qt_help_*"

ENDLOCAL
