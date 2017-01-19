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

git rev-parse --short HEAD > current-sha.txt
set /p SHA=<current-sha.txt
del current-sha.txt

set YYYYMMDD=%date:~10,4%%date:~4,2%%date:~7,2%
set BASENAME=%1

REM TODO(XXX): Reduce code duplication for creating and building with a subroutine. 
echo Creating environment %BASENAME%-%YYYYMMDD%-x86-release-static-%SHA%
call create_environment.bat %BASENAME%-%YYYYMMDD%-x86-release-static-%SHA% >create-%BASENAME%-%YYYYMMDD%-x86-release-static-%SHA%.log 2>&1
if ERRORLEVEL 1 (
  echo Creating x86 Release environment failed.
  ENDLOCAL
  exit /b 1
)

echo Creating environment %BASENAME%-%YYYYMMDD%-x64-release-static-%SHA%
call create_environment.bat %BASENAME%-%YYYYMMDD%-x64-release-static-%SHA% >create-%BASENAME%-%YYYYMMDD%-x64-release-static-%SHA%.log 2>&1
if ERRORLEVEL 1 (
  echo Creating x64 Release environment failed.
  ENDLOCAL
  exit /b 1
)

echo Creating environment %BASENAME%-%YYYYMMDD%-x86-release-fastbuild-static-%SHA%
call create_environment.bat %BASENAME%-%YYYYMMDD%-x86-release-fastbuild-static-%SHA% >create-%BASENAME%-%YYYYMMDD%-x86-release-fastbuild-static-%SHA%.log 2>&1
if ERRORLEVEL 1 (
  echo Creating x86 ReleaseFastbuild environment failed.
  ENDLOCAL
  exit /b 1
)

echo Creating environment %BASENAME%-%YYYYMMDD%-x64-release-fastbuild-static-%SHA%
call create_environment.bat %BASENAME%-%YYYYMMDD%-x64-release-fastbuild-static-%SHA% >create-%BASENAME%-%YYYYMMDD%-x64-release-fastbuild-static-%SHA%.log 2>&1
if ERRORLEVEL 1 (
  echo Creating x64 ReleaseFastbuild environment failed.
  ENDLOCAL
  exit /b 1
)

REM echo Creating environment %BASENAME%-%YYYYMMDD%-x86-debug-static-%SHA%
REM call create_environment.bat %BASENAME%-%YYYYMMDD%-x86-debug-static-%SHA% 2>&1 >create-%BASENAME%-%YYYYMMDD%-x86-debug-static-%SHA%.log
REM if ERRORLEVEL 1 (
REM   echo Creating x86 Debug environment failed.
REM   ENDLOCAL
REM   exit /b 1
REM )
REM echo Creating environment %BASENAME%-%YYYYMMDD%-x64-debug-static-%SHA%
REM call create_environment.bat %BASENAME%-%YYYYMMDD%-x64-debug-static-%SHA% 2>&1 >create-%BASENAME%-%YYYYMMDD%-x64-debug-static-%SHA%.log
REM if ERRORLEVEL 1 (
REM   echo Creating x64 Debug environment failed.
REM   ENDLOCAL
REM   exit /b 1
REM )

echo Building x86 Release
cd %BASENAME%-%YYYYMMDD%-x86-release-static-%SHA%
call build_environment.bat x86 Release >build_environment.log 2>&1
if ERRORLEVEL 1 (
  echo Building x86 Release failed.
  ENDLOCAL
  exit /b 1
)
cd ..

echo Building x64 Release
cd %BASENAME%-%YYYYMMDD%-x64-release-static-%SHA%
call build_environment.bat x64 Release >build_environment.log 2>&1
if ERRORLEVEL 1 (
  echo Building x64 Release failed.
  ENDLOCAL
  exit /b 1
)
cd ..

echo Building x86 ReleaseFastbuild
cd %BASENAME%-%YYYYMMDD%-x86-release-fastbuild-static-%SHA%
call build_environment.bat x86 ReleaseFastbuild >build_environment.log 2>&1
if ERRORLEVEL 1 (
  echo Building x86 ReleaseFastbuild failed.
  ENDLOCAL
  exit /b 1
)
cd ..

echo Building x64 RelaseFastbuild
cd %BASENAME%-%YYYYMMDD%-x64-release-fastbuild-static-%SHA%
call build_environment.bat x64 ReleaseFastbuild >build_environment.log 2>&1
if ERRORLEVEL 1 (
  echo Building x64 ReleaseFastbuild failed.
  ENDLOCAL
  exit /b 1
)
cd ..

REM echo Building x86 Debug
REM cd %BASENAME%-%YYYYMMDD%-x86-debug-static-%SHA%
REM call build_environment.bat x86 Debug >build_environment.log 2>&1
REM if ERRORLEVEL 1 (
  REM echo Building x86 Debug failed.
  REM ENDLOCAL
  REM exit /b 1
REM )
REM cd ..

REM echo Building x64 Debug
REM cd %BASENAME%-%YYYYMMDD%-x64-debug-static-%SHA%
REM call build_environment.bat x64 Debug >build_environment.log 2>&1
REM if ERRORLEVEL 1 (
  REM echo Building x64 Debug failed.
  REM ENDLOCAL
  REM exit /b 1
REM )
REM cd ..

ENDLOCAL
