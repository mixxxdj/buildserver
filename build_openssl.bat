SETLOCAL
echo ---- Building OpenSSL ----
set OPENSSL_PATH=openssl-1.0.2h
SET VALRETURN=0

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=Release
) else (
  set CONFIG=Debug
)

cd build\%OPENSSL_PATH%
if %PLATFORM% == x64 (
  rem From https://wiki.openssl.org/index.php/Compilation_and_Installation#Windows
  perl Configure VC-WIN64A
  call ms\do_win64a.bat
) else (
  perl Configure VC-WIN32
  rem NASM bust be in the PATH
  call ms\do_nasm
  rem Use the below instead if you don't want the assembly language files
  rem perl Configure VC-WIN32 no-asm
  rem ms\do_ms
)
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

echo Cleaning both...
nmake -f ms\ntdll.mak clean
nmake -f ms\nt.mak clean
REM IGNORING ERRORS on either of these, because if the directory does not exist, they fail with
REM the del command and would make the compilation stop.

echo Building...
if %STATIC_LIBS% (
  nmake -f ms\nt.mak
) else (
  nmake -f ms\ntdll.mak
)
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

set OPENSSL_TARGETS=out32
if NOT %STATIC_LIBS% ( 
set OPENSSL_TARGETS=%OPENSSL_TARGETS%dll
)
REM if NOT %CONFIG_RELEASE% (
REM set OPENSSL_TARGETS=%OPENSSL_TARGETS%.dbg
REM )

copy /b %OPENSSL_TARGETS%\libeay32.lib %LIB_DIR%
copy /b %OPENSSL_TARGETS%\ssleay32.lib %LIB_DIR%
if NOT %STATIC_LIBS% (
  copy /b %OPENSSL_TARGETS%\libeay32.dll %LIB_DIR%
  copy /b %OPENSSL_TARGETS%\ssleay32.dll %LIB_DIR%
)
md %INCLUDE_DIR%\openssl
copy /b inc32\openssl\*.h %INCLUDE_DIR%\openssl\

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%