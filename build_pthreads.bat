SETLOCAL
echo ---- Building pthreads ----
set PTHREADS_PATH=pthreads-2.9.1
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

cd build\%PTHREADS_PATH%
rem TODO(Pegasus): Add PTW32_STATIC_LIB for static builds without clobbering the others defined in the project files
%MSBUILD% pthread.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:pthread:Clean;pthread:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

if NOT %STATIC_LIBS% ( copy %PLATFORM%\%CONFIG%\pthread.dll %LIB_DIR% )
copy %PLATFORM%\%CONFIG%\pthread.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\pthread.pdb %LIB_DIR%
copy pthread.h %INCLUDE_DIR%
copy semaphore.h %INCLUDE_DIR%
copy sched.h %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%