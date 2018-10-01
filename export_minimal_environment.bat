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

REM set QT4_VERSION=4.8.7
REM set "QT4_DIR=%ENVIRONMENT_PATH%\build\qt-everywhere-opensource-src-%QT4_VERSION%"
REM set "QT4_MINIMAL_DIR=%MINIMAL_PATH%\Qt-%QT4_VERSION%\"

REM md "%QT4_MINIMAL_DIR%"

REM only copy binaries we need
REM md "%QT4_MINIMAL_DIR%\bin"
REM %COPY% "%QT4_DIR%\bin\lconvert.exe"  "%QT4_MINIMAL_DIR%\bin"
REM %COPY% "%QT4_DIR%\bin\lrelease.exe"  "%QT4_MINIMAL_DIR%\bin"
REM %COPY% "%QT4_DIR%\bin\lupdate.exe"  "%QT4_MINIMAL_DIR%\bin"
REM %COPY% "%QT4_DIR%\bin\moc.exe"  "%QT4_MINIMAL_DIR%\bin"
REM %COPY% "%QT4_DIR%\bin\rcc.exe"  "%QT4_MINIMAL_DIR%\bin"
REM %COPY% "%QT4_DIR%\bin\uic.exe"  "%QT4_MINIMAL_DIR%\bin"

REM %XCOPY% "%QT4_DIR%\include" "%QT4_MINIMAL_DIR%\include"

REM manually copy only libs Mixxx uses
REM md "%QT4_MINIMAL_DIR%\lib"
REM %COPY% "%QT4_DIR%\lib\qtmain.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtCore.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtGui.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtNetwork.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtOpenGL.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtScript.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtScriptTools.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtSql.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtSvg.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtTest.lib"  "%QT4_MINIMAL_DIR%\lib\"
REM %COPY% "%QT4_DIR%\lib\QtXml.lib"  "%QT4_MINIMAL_DIR%\lib\"


REM %XCOPY% "%QT4_DIR%\plugins" "%QT4_MINIMAL_DIR%\plugins"

REM only copy header files in src.
REM %XCOPY% "%QT4_DIR%\src\*.h" "%QT4_MINIMAL_DIR%\src"

REM manually copy PDBs
REM TODO(rryan) Qt5 offers a "release with debug info" option.
REM %COPY% "%QT4_DIR%\src\corelib\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtCore.pdb"
REM skipping declarative -- add it back in for Qt5.
REM %COPY% "%QT4_DIR%\src\gui\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtGui.pdb"
REM %COPY% "%QT4_DIR%\src\network\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtNetwork.pdb"
REM %COPY% "%QT4_DIR%\src\opengl\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtOpenGL.pdb"
REM %COPY% "%QT4_DIR%\src\plugins\accessible\widgets\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\accessible\qtaccessiblewidgets.pdb"
REM %COPY% "%QT4_DIR%\src\plugins\iconengines\svgiconengine\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\iconengines\qsvgicon.pdb"
REM %COPY% "%QT4_DIR%\src\plugins\imageformats\ico\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\imageformats\qico.pdb"
REM %COPY% "%QT4_DIR%\src\plugins\imageformats\svg\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\imageformats\qsvg.pdb"
REM %COPY% "%QT4_DIR%\src\plugins\imageformats\tga\vc140.pdb"  "%QT4_MINIMAL_DIR%\plugins\imageformats\qtga.pdb"
REM %COPY% "%QT4_DIR%\src\script\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtScript.pdb"
REM %COPY% "%QT4_DIR%\src\scripttools\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtScriptTools.pdb"
REM %COPY% "%QT4_DIR%\src\sql\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtSql.pdb"
REM %COPY% "%QT4_DIR%\src\svg\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtSvg.pdb"
REM %COPY% "%QT4_DIR%\src\testlib\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtTest.pdb"
REM %COPY% "%QT4_DIR%\src\xml\vc140.pdb"  "%QT4_MINIMAL_DIR%\lib\QtXml.pdb"

REM only copy compiled QT4_ translations.
REM %XCOPY% "%QT4_DIR%\translations\QT4_*.qm" "%QT4_MINIMAL_DIR%\translations"
REM The above also copies QT4_help_*, which we don't need.
REM del "%QT4_MINIMAL_DIR%\translations\QT4_help_*"

set QT5_VERSION=5.11.2
%XCOPY% "%ENVIRONMENT_PATH%\Qt-%QT5_VERSION%" "%MINIMAL_PATH%\Qt-%QT5_VERSION%\"

ENDLOCAL
