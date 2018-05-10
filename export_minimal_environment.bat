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

set QT4_VERSION=4.8.7
set "QT4_DIR=%ENVIRONMENT_PATH%\build\qt-everywhere-opensource-src-%QT4_VERSION%"
set "QT4_MINIMAL_DIR=%MINIMAL_PATH%\Qt-%QT4_VERSION%\"

md "%QT4_MINIMAL_DIR%"

REM only copy binaries we need
md "%QT4_MINIMAL_DIR%\bin"
%COPY% "%QT4_DIR%\bin\lconvert.exe"  "%QT4_MINIMAL_DIR%\bin"
%COPY% "%QT4_DIR%\bin\lrelease.exe"  "%QT4_MINIMAL_DIR%\bin"
%COPY% "%QT4_DIR%\bin\lupdate.exe"  "%QT4_MINIMAL_DIR%\bin"
%COPY% "%QT4_DIR%\bin\moc.exe"  "%QT4_MINIMAL_DIR%\bin"
%COPY% "%QT4_DIR%\bin\rcc.exe"  "%QT4_MINIMAL_DIR%\bin"
%COPY% "%QT4_DIR%\bin\uic.exe"  "%QT4_MINIMAL_DIR%\bin"

%XCOPY% "%QT4_DIR%\include" "%QT4_MINIMAL_DIR%\include"

REM manually copy only libs Mixxx uses
md "%QT4_MINIMAL_DIR%\lib"
%COPY% "%QT4_DIR%\lib\qtmain.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtCore.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtGui.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtNetwork.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtOpenGL.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtScript.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtScriptTools.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtSql.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtSvg.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtTest.lib"  "%QT4_MINIMAL_DIR%\lib\"
%COPY% "%QT4_DIR%\lib\QtXml.lib"  "%QT4_MINIMAL_DIR%\lib\"


%XCOPY% "%QT4_DIR%\plugins" "%QT4_MINIMAL_DIR%\plugins"

REM only copy header files in src.
%XCOPY% "%QT4_DIR%\src\*.h" "%QT4_MINIMAL_DIR%\src"

REM manually copy PDBs
REM TODO(rryan) Qt5 offers a "release with debug info" option.
%COPY% "%QT4_DIR%\src\corelib\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtCore.pdb"
REM skipping declarative -- add it back in for Qt5.
%COPY% "%QT4_DIR%\src\gui\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtGui.pdb"
%COPY% "%QT4_DIR%\src\network\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtNetwork.pdb"
%COPY% "%QT4_DIR%\src\opengl\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtOpenGL.pdb"
%COPY% "%QT4_DIR%\src\plugins\accessible\widgets\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\accessible\qtaccessiblewidgets.pdb"
%COPY% "%QT4_DIR%\src\plugins\iconengines\svgiconengine\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\iconengines\qsvgicon.pdb"
%COPY% "%QT4_DIR%\src\plugins\imageformats\ico\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\imageformats\qico.pdb"
%COPY% "%QT4_DIR%\src\plugins\imageformats\svg\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\imageformats\qsvg.pdb"
%COPY% "%QT4_DIR%\src\plugins\imageformats\tga\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\imageformats\qtga.pdb"
%COPY% "%QT4_DIR%\src\script\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtScript.pdb"
%COPY% "%QT4_DIR%\src\scripttools\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtScriptTools.pdb"
%COPY% "%QT4_DIR%\src\sql\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtSql.pdb"
%COPY% "%QT4_DIR%\src\svg\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtSvg.pdb"
%COPY% "%QT4_DIR%\src\testlib\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtTest.pdb"
%COPY% "%QT4_DIR%\src\xml\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtXml.pdb"

REM only copy compiled QT4_ translations.
%XCOPY% "%QT4_DIR%\translations\QT4_*.qm" "%QT4_MINIMAL_DIR%\translations"
REM The above also copies QT4_help_*, which we don't need.
del "%QT4_MINIMAL_DIR%\translations\QT4_help_*"

set QT5_VERSION=5.10.1
%XCOPY% "%ENVIRONMENT_PATH%\Qt-%QT5_VERSION%" "%MINIMAL_PATH%\Qt-%QT5_VERSION%\"

ENDLOCAL
