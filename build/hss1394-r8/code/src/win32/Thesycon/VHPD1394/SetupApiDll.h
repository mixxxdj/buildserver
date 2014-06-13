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

#ifndef _SetupApiDll_h_
#define _SetupApiDll_h_

// unicode is not supported
#ifdef UNICODE
#undef UNICODE
#endif

// In Borland C++, we have to force byte alignment for setupapi.h.
// This is necessary because there seems to be a problem with the
// implementations of pshpackX.h and poppack.h.
#ifdef __BORLANDC__
#pragma pack(1)
#endif

// get setup API functions (only available in Win98 and later systems)
#include <setupapi.h>

// restore previous settings
#ifdef __BORLANDC__
#pragma pack()
#endif



//
// CSetupApiDll
//
class CSetupApiDll
{
public:

  // standard constructor
  CSetupApiDll();

  // destructor
  ~CSetupApiDll();


  // load setupapi.dll
  // returns TRUE if successful, FALSE otherwise
  BOOL Load(void);

  // release setupapi.dll
  void Release(void);


// implementation
protected:
  
  // init function pointers
  void InitPointers(void);


  // We have to repeat some prototypes from setupapi.h.
  // Unfortunately, the .h file does not define types for the functions.

  typedef
  WINSETUPAPI
  HDEVINFO 
  WINAPI
  F_SetupDiGetClassDevsA(
    CONST GUID * ClassGuid, 
    PCSTR *Enumerator, 
    HWND hwndParent, 
    DWORD Flags
    );

  typedef 
  WINSETUPAPI
  BOOL
  WINAPI
  F_SetupDiDestroyDeviceInfoList(
    HDEVINFO DeviceInfoSet
    );

  typedef 
  WINSETUPAPI
  BOOL 
  WINAPI
  F_SetupDiEnumDeviceInterfaces(
    HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData, 
    CONST GUID * InterfaceClassGuid,
    DWORD MemberIndex,
    PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    );

  typedef
  WINSETUPAPI 
  BOOL 
  WINAPI
  F_SetupDiGetDeviceInterfaceDetailA(
    HDEVINFO DeviceInfoSet,
    PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    PSP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData, 
    DWORD DeviceInterfaceDetailDataSize,
    PDWORD RequiredSize, 
    PSP_DEVINFO_DATA DeviceInfoData 
    );

  // handle for DLL
  HMODULE mDllHandle;


  // function pointers
  // We make the pointers public to simplify access by external code.
public:
  F_SetupDiGetClassDevsA*              SetupDiGetClassDevs;
  F_SetupDiDestroyDeviceInfoList*      SetupDiDestroyDeviceInfoList;
  F_SetupDiEnumDeviceInterfaces*       SetupDiEnumDeviceInterfaces;
  F_SetupDiGetDeviceInterfaceDetailA*  SetupDiGetDeviceInterfaceDetail;

}; // class CSetupApiDll

#endif // _SetupApiDll_h_

/*************************** EOF **************************************/
