echo --- Building SQLite3 ----
set SQLITE_PATH=sqlite-amalgamation-3130000

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

cd build\%SQLITE_PATH%

%MSBUILD% sqlite3.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:sqlite3:Clean;sqlite3:Rebuild

copy %PLATFORM%\%CONFIG%\sqlite3.lib %LIB_DIR%
copy %PLATFORM%\%CONFIG%\sqlite3.dll %LIB_DIR%
copy %PLATFORM%\%CONFIG%\sqlite3.pdb %LIB_DIR%
copy sqlite3.h %INCLUDE_DIR%

cd %ROOT_DIR%