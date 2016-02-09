- Specify minimum platform version to be 5.01 for x86 and 5.02 for x64
- Use /MT for release and /MTd for debug
- Use v120_xp (or latest_xp) for platform toolset
- Use /subsystem:WINDOWS
- Use /Zi and define the PDB output filename as "$(OutDir)$(TargetName).pdb" (for ease of scripting the copy in build_xxx.bat)
- Use /O2
- /fp:fast if appropriate (skipped protobuf, SQLite, zlib, taglib, pthreads
    and chromaprint since inaccuracies related to compression and data
    serialization seem like a bad idea)
- /LTCG for Release build
