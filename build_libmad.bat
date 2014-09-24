echo "Building libmad"
set MAD_PATH=libmad-0.15.1b

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

cd build\%MAD_PATH%\msvc++
%MSBUILD% libmad.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:libmad:Clean;libmad:Rebuild

REM TODO(rryan): Currently generating a static library only.
copy %PLATFORM%\%CONFIG%\libmad.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\libmad.pdb %LIB_DIR%
REM copy %PLATFORM%\%CONFIG%\libmad.dll %LIB_DIR%
REM TODO(rryan): mad.h defines FPM_INTEL.
copy ..\mad.h %INCLUDE_DIR% 

cd %ROOT_DIR%