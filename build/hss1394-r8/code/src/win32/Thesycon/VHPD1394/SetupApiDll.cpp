/************************************************************************
 *
 *  Module:       SetupApiDll.h
 *  Long name:    CSetupApiDll class
 *  Description:  handling of the setupapi.dll
 *
 *  Runtime Env.: Win32
 *  Author(s):    Guenter Hildebrandt, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

// for shorter and faster windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
// unicode is not supported
#ifdef UNICODE
#undef UNICODE
#endif

#include <windows.h>
#include "SetupApiDll.h"


// standard constructor
CSetupApiDll::CSetupApiDll()
{
  mDllHandle = NULL;
  InitPointers();
}


// destructor
CSetupApiDll::~CSetupApiDll()
{
  Release();
}


// init function pointers
void CSetupApiDll::InitPointers(void)
{
  SetupDiGetClassDevs = NULL;
  SetupDiDestroyDeviceInfoList = NULL;
  SetupDiEnumDeviceInterfaces = NULL;
  SetupDiGetDeviceInterfaceDetail = NULL;
}



// load setupapi.dll
BOOL CSetupApiDll::Load(void)
{
  if ( mDllHandle!=NULL ) {
    // library is already loaded
    return TRUE;
  }

  // try to load the DLL
  mDllHandle = ::LoadLibrary("setupapi.dll");
  if ( mDllHandle==NULL ) {
    // failed to load library
    return FALSE;
  }
  // DLL successfully loaded
  BOOL succ = TRUE;
  
  // try to get the function pointers we need
  SetupDiGetClassDevs = (F_SetupDiGetClassDevsA*)::GetProcAddress(mDllHandle,"SetupDiGetClassDevsA");
  if ( SetupDiGetClassDevs==NULL ) succ = FALSE;

  SetupDiDestroyDeviceInfoList = (F_SetupDiDestroyDeviceInfoList*)::GetProcAddress(mDllHandle,"SetupDiDestroyDeviceInfoList");
  if ( SetupDiDestroyDeviceInfoList==NULL ) succ = FALSE;

  SetupDiEnumDeviceInterfaces = (F_SetupDiEnumDeviceInterfaces*)::GetProcAddress(mDllHandle,"SetupDiEnumDeviceInterfaces");
  if ( SetupDiEnumDeviceInterfaces==NULL ) succ = FALSE;

  SetupDiGetDeviceInterfaceDetail = (F_SetupDiGetDeviceInterfaceDetailA*)::GetProcAddress(mDllHandle,"SetupDiGetDeviceInterfaceDetailA");
  if ( SetupDiGetDeviceInterfaceDetail==NULL ) succ = FALSE;

  // all pointers ok?
  if ( !succ ) {
    // no, release DLL
    Release();
  }
  
  return succ;
}



// release setupapi.dll
void CSetupApiDll::Release(void)
{
  if ( mDllHandle!=NULL ) {
    ::FreeLibrary(mDllHandle);
    mDllHandle = NULL;
    InitPointers();
  }
}





/*************************** EOF **************************************/
