- Set Output Directory to $(SolutionDir)$(Platform)\$(Configuration)\
- Set Intermediate directory to $(Platform)\$(Configuration)\
- Specify minimum platform version to be 5.01 for x86 and 5.02 for x64
- Use /MD for release and /MDd for debug
- Use v<latest>_xp for platform toolset
- Use /subsystem:WINDOWS
- Use /Zi and define the PDB output filename as $(OutDir)$(TargetName).pdb
    (for ease of scripting the copy in build_xxx.bat)
- Use /O2 on Release build
- /fp:fast if appropriate (skipped protobuf, SQLite, zlib, taglib, pthreads
    and chromaprint since inaccuracies related to compression and data
    serialization seem like a bad idea)
- /GL on Compiler and /LTCG on Linker for Release build
  (Whole Program Optimization in General)
- /MP to compile faster on multi-core system