echo "Building protobuf"
set PROTOBUF_PATH=protobuf-2.5.0

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

copy %PLATFORM%\%CONFIG%\libprotobuf-lite.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libprotobuf-lite.lib %LIB_DIR%
call extract_includes.bat 
xcopy /E /Y include %INCLUDE_DIR%

cd %ROOT_DIR%