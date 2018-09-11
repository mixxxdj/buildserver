SETLOCAL
echo ---- Building FFTW ----
set FFTW_PATH=fftw-3.3.8
SET VALRETURN=0

if %MACHINE_X86% (
  set PLATFORM=Win32
) else (
  set PLATFORM=x64
)

if %CONFIG_RELEASE% (
  set CONFIG=Release
  if %STATIC_LIBS% ( 
    set CONFIG=Static-Release
  )
) else (
  set CONFIG=Debug
  if %STATIC_LIBS% ( 
    set CONFIG=Static-Debug
  )
)

REM TODO(XXX): The output paths differ based on platform.
if %MACHINE_X86% (
  set OUTPUT_PATH=%CONFIG%
) else (
  set OUTPUT_PATH=%PLATFORM%\%CONFIG%
)

cd build\%FFTW_PATH%\fftw-3.3-libs

echo Cleaning all...
%MSBUILD% fftw-3.3-libs.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Clean
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

REM NOTE(rryan): Couldn't get solution itself to resolve the build actions. Odd.
cd libfftw-3.3
%MSBUILD% libfftw-3.3.vcxproj /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy %PLATFORM%\%CONFIG%\libfftw-3.3.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libfftw-3.3.pdb %LIB_DIR%
if not %STATIC_LIBS% ( 
  copy %PLATFORM%\%CONFIG%\libfftw-3.3.dll %LIB_DIR%
) 

rem Need libfftwf as well
cd ..\libfftwf-3.3
%MSBUILD% libfftwf-3.3.vcxproj /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Rebuild
IF ERRORLEVEL 1 (
SET VALRETURN=1
goto END
)

copy %OUTPUT_PATH%\libfftwf-3.3.lib %LIB_DIR%
copy %OUTPUT_PATH%\libfftwf-3.3.pdb %LIB_DIR%
if not %STATIC_LIBS% (
  copy %OUTPUT_PATH%\libfftwf-3.3.dll %LIB_DIR%
) 

copy ..\..\api\fftw3.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%
