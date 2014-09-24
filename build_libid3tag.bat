echo "Building libid3tag"
set ID3TAG_PATH=libid3tag-0.15.1b

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

cd build\%ID3TAG_PATH%\msvc++
%MSBUILD% libid3tag.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libid3tag:Clean;libid3tag:Rebuild

REM TODO(rryan): Currently generating a static library only.
copy %PLATFORM%\%CONFIG%\libid3tag.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libid3tag.pdb %LIB_DIR%
REM copy %PLATFORM%\%CONFIG%\libid3tag.dll %LIB_DIR%
copy ..\id3tag.h %INCLUDE_DIR% 

cd %ROOT_DIR%