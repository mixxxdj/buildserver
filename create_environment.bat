@echo off
SETLOCAL

set XCOPY=xcopy /Y /I /E /Q
set ROBOCOPY=robocopy /E /nfl /ndl /njh /njs
set "ENVIRONMENTS_DIR=%CD%"
set "ENVIRONMENT_NAME=%1"
set "ENVIRONMENT_PATH=%CD%\%1"

if exist "%ENVIRONMENT_PATH%" (
  echo.
  echo Directory "%ENVIRONMENT_PATH%" already exists.
  ENDLOCAL
  exit /b 1
)

if not exist ASIOSDK2.3 (
  echo.
  echo You need to obtain and copy the ASIO2.3 SDK to the folder:
  echo %CD%
  echo http://www.steinberg.net/en/company/developer.html
  ENDLOCAL
  exit /b 1
)

REM if not exist qt-everywhere-opensource-src-4.8.7 (
REM  echo.
REM  echo You need to obtain and unzip qt-everywhere-opensource-src-4.8.7.zip to the folder:
REM  echo %CD%
REM  echo https://download.qt.io/official_releases/qt/4.8/4.8.7/qt-everywhere-opensource-src-4.8.7.zip
REM  ENDLOCAL
REM  exit /b 1
REM )

if not exist qt-everywhere-src-5.10.1 (
  echo.
  echo You need to obtain and unzip qt-everywhere-src-5.10.1.zip to the folder:
  echo %CD%
  echo https://download.qt.io/official_releases/qt/5.10/5.10.1/single/qt-everywhere-src-5.10.1.zip
  ENDLOCAL
  exit /b 1
)

if not exist vc_redist.x86.exe (
  echo.
  echo You need to obtain the Microsoft Visual C++ 2015 Redistributable and copy vc_redist.x86.exe and vc_redist.x64.exe to this folder:
  echo %CD%
  echo https://www.microsoft.com/en-us/download/details.aspx?id=53587
  ENDLOCAL
  exit /b 1
)

git clone . "%ENVIRONMENT_NAME%"

echo Copying VC redistributable installers.
copy vc_redist* "%ENVIRONMENT_PATH%"
IF ERRORLEVEL 1 (
echo Copying Visual C++ redistributable installers failed.
ENDLOCAL
exit /b 1
)

echo Copying ASIO SDK.
%XCOPY% ASIOSDK2.3 "%ENVIRONMENT_PATH%\build\pa_stable_v190600_20161030\src\hostapi\asio\ASIOSDK"
IF ERRORLEVEL 1 (
echo Copying ASIO SDK failed.
ENDLOCAL
exit /b 1
)

echo Copying Qt4
REM Note that using xcopy to copy Qt can run into "Insufficient memory" errors
REM due to path lengths growing larger than 254 characters.
%ROBOCOPY% qt-everywhere-opensource-src-4.8.7 "%ENVIRONMENT_PATH%\build\qt-everywhere-opensource-src-4.8.7"
REM robocopy errorlevel 1 means files were copied.
IF ERRORLEVEL 2 (
echo Copying Qt 4.8.7 failed.
ENDLOCAL
exit /b 1
)

echo Copying Qt5
REM Note that using xcopy to copy Qt can run into "Insufficient memory" errors
REM due to path lengths growing larger than 254 characters.
%ROBOCOPY% qt-everywhere-src-5.10.1 "%ENVIRONMENT_PATH%\build\Q5101"
REM robocopy errorlevel 1 means files were copied.
IF ERRORLEVEL 2 (
echo Copying Qt 5.10.1 failed.
ENDLOCAL
exit /b 1
)

echo Success.
exit /b 0
ENDLOCAL
