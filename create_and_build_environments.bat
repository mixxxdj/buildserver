@echo off
SETLOCAL

if "%1" == "" (
  echo Missing parameters. Usage:
  echo.
  echo create_environments.bat ^<environment base-name^>
  echo.
  echo Example: create_and_build_environments.bat 2.1
  echo Creates the following environments:
  echo   2.1-YYYYMMDD-x86-release-static-gitsha
  echo   2.1-YYYYMMDD-x64-release-static-gitsha
  echo   2.1-YYYYMMDD-x86-release-fastbuild-static-gitsha
  echo   2.1-YYYYMMDD-x64-release-fastbuild-static-gitsha
  echo   2.1-YYYYMMDD-x86-debug-static-gitsha
  echo   2.1-YYYYMMDD-x64-debug-static-gitsha
  exit /b 1
)

git rev-parse HEAD > current-sha.txt
set /p SHA=<current-sha.txt
del current-sha.txt

set YYYYMMDD=%date:~10,4%%date:~4,2%%date:~7,2%
set BASENAME=%1

call create_environment.bat %BASENAME%-%YYYYMMDD%-x86-release-static-%SHA%
if ERRORLEVEL 1 (
  echo Creating x86 Release environment failed.
  ENDLOCAL
  exit /b 1
)

call create_environment.bat %BASENAME%-%YYYYMMDD%-x64-release-static-%SHA%
if ERRORLEVEL 1 (
  echo Creating x64 Release environment failed.
  ENDLOCAL
  exit /b 1
)

call create_environment.bat %BASENAME%-%YYYYMMDD%-x86-release-fastbuild-static-%SHA%
if ERRORLEVEL 1 (
  echo Creating x86 ReleaseFastbuild environment failed.
  ENDLOCAL
  exit /b 1
)

call create_environment.bat %BASENAME%-%YYYYMMDD%-x64-release-fastbuild-static-%SHA%
if ERRORLEVEL 1 (
  echo Creating x64 ReleaseFastbuild environment failed.
  ENDLOCAL
  exit /b 1
)

REM call create_environment.bat %BASENAME%-%YYYYMMDD%-x86-debug-static-%SHA%
REM if ERRORLEVEL 1 (
REM   echo Creating x86 Debug environment failed.
REM   ENDLOCAL
REM   exit /b 1
REM )
REM call create_environment.bat %BASENAME%-%YYYYMMDD%-x64-debug-static-%SHA%
REM if ERRORLEVEL 1 (
REM   echo Creating x64 Debug environment failed.
REM   ENDLOCAL
REM   exit /b 1
REM )

echo Building x86 Release
cd %BASENAME%-%YYYYMMDD%-x86-release-static-%SHA%
call build_environment.bat x86 Release
if ERRORLEVEL 1 (
  echo Building x86 Release failed.
  ENDLOCAL
  exit /b 1
)
cd ..

echo Building x64 Release
cd %BASENAME%-%YYYYMMDD%-x64-release-static-%SHA%
call build_environment.bat x64 Release
if ERRORLEVEL 1 (
  echo Building x64 Release failed.
  ENDLOCAL
  exit /b 1
)
cd ..

echo Building x86 ReleaseFastbuild
cd %BASENAME%-%YYYYMMDD%-x86-release-fastbuild-static-%SHA%
call build_environment.bat x86 ReleaseFastbuild
if ERRORLEVEL 1 (
  echo Building x86 ReleaseFastbuild failed.
  ENDLOCAL
  exit /b 1
)
cd ..

echo Building x64 RelaseFastbuild
cd %BASENAME%-%YYYYMMDD%-x64-release-fastbuild-static-%SHA%
call build_environment.bat x64 ReleaseFastbuild
if ERRORLEVEL 1 (
  echo Building x64 ReleaseFastbuild failed.
  ENDLOCAL
  exit /b 1
)
cd ..

REM echo Building x86 Debug
REM cd %BASENAME%-%YYYYMMDD%-x86-debug-static-%SHA%
REM call build_environment.bat x86 Debug
REM if ERRORLEVEL 1 (
  REM echo Building x86 Debug failed.
  REM ENDLOCAL
  REM exit /b 1
REM )
REM cd ..

REM echo Building x64 Debug
REM cd %BASENAME%-%YYYYMMDD%-x64-debug-static-%SHA%
REM call build_environment.bat x64 Debug
REM if ERRORLEVEL 1 (
  REM echo Building x64 Debug failed.
  REM ENDLOCAL
  REM exit /b 1
REM )
REM cd ..

ENDLOCAL
