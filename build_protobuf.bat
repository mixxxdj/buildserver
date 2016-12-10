SETLOCAL
echo "Building protobuf"
set PROTOBUF_PATH=protobuf-2.6.1
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

cd build\%PROTOBUF_PATH%\vsprojects
%MSBUILD% protobuf.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libprotobuf-lite:Clean;libprotobuf-lite:Rebuild
IF ERRORLEVEL 1 (
    SET VALRETURN=1
	goto END
)

copy %PLATFORM%\%CONFIG%\libprotobuf-lite.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libprotobuf-lite.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libprotobuf-lite.pdb %LIB_DIR%
call extract_includes.bat 
xcopy /E /Y include %INCLUDE_DIR%

:END
cd %ROOT_DIR%
REM the GOTO command resets the errorlevel and the endlocal resets the local environment,
REM so I have to use this workaround
ENDLOCAL & SET VALRETURN=%VALRETURN%
exit /b %VALRETURN%