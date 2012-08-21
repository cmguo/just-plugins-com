//////////////////////////////////////////////////////////////////////////
//
// dllmain.cpp : Implements DLL exports and COM class factory
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Note: This source file implements the class factory for the sample
//       media source, plus the following DLL functions:
//       - DllMain
//       - DllCanUnloadNow
//       - DllRegisterServer
//       - DllUnregisterServer
//       - DllGetClassObject
//
//////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <assert.h>
#include <strsafe.h>

#include <initguid.h>

#include <new>

#include "MediaSdk.h"

const DWORD CHARS_IN_GUID = 39;


HRESULT RegisterObject(
    HMODULE hModule,
    const GUID& guid,
    const TCHAR *pszDescription,
    const TCHAR *pszThreadingModel
    );

HRESULT UnregisterObject(const GUID& guid);


// {D8D232A7-EBC9-4c24-9429-BCB114EB13C8}
DEFINE_GUID(CLSID_PpboxMediaSdk, 
    0xd8d232a7, 0xebc9, 0x4c24, 0x94, 0x29, 0xbc, 0xb1, 0x14, 0xeb, 0x13, 0xc8);

const TCHAR* sDescription = TEXT("Ppbox MediaSdk");

// Module Ref count
long g_cRefModule = 0;

// Handle to the DLL's module
HMODULE g_hModule = NULL;

void DllAddRef()
{
    InterlockedIncrement(&g_cRefModule);
}

void DllRelease()
{
    InterlockedDecrement(&g_cRefModule);
}

// Misc Registry helpers
HRESULT SetKeyValue(HKEY hKey, const TCHAR *sName, const TCHAR *sValue);


//
// IClassFactory implementation
//

typedef HRESULT (*PFNCREATEINSTANCE)(REFIID riid, void **ppvObject);
struct CLASS_OBJECT_INIT
{
    const CLSID *pClsid;
    PFNCREATEINSTANCE pfnCreate;
};

// Classes supported by this module:
const CLASS_OBJECT_INIT c_rgClassObjectInit[] =
{
    { &CLSID_PpboxMediaSdk, MediaSdk_CreateInstance },
};

class CClassFactory : public IClassFactory
{
public:

    static HRESULT CreateInstance(
        REFCLSID clsid,                                 // The CLSID of the object to create (from DllGetClassObject)
        const CLASS_OBJECT_INIT *pClassObjectInits,     // Array of class factory data.
        size_t cClassObjectInits,                       // Number of elements in the array.
        REFIID riid,                                    // The IID of the interface to retrieve (from DllGetClassObject)
        void **ppv                                      // Receives a pointer to the interface.
        )
    {
        *ppv = NULL;

        HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

        for (size_t i = 0; i < cClassObjectInits; i++)
        {
            if (clsid == *pClassObjectInits[i].pClsid)
            {
                IClassFactory *pClassFactory = new (std::nothrow) CClassFactory(pClassObjectInits[i].pfnCreate);

                if (pClassFactory)
                {
                    hr = pClassFactory->QueryInterface(riid, ppv);
                    pClassFactory->Release();
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                break; // match found
            }
        }
        return hr;
    }

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == IID_IClassFactory) {
            *ppv = (IClassFactory *)this;
            AddRef();
            return S_OK;
        };
        return E_FAIL;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    // IClassFactory methods

    STDMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
    {
        return punkOuter ? CLASS_E_NOAGGREGATION : m_pfnCreate(riid, ppv);
    }

    STDMETHODIMP LockServer(BOOL fLock)
    {
        if (fLock)
        {
            DllAddRef();
        }
        else
        {
            DllRelease();
        }
        return S_OK;
    }

private:

    CClassFactory(PFNCREATEINSTANCE pfnCreate) : m_cRef(1), m_pfnCreate(pfnCreate)
    {
        DllAddRef();
    }

    ~CClassFactory()
    {
        DllRelease();
    }

    long m_cRef;
    PFNCREATEINSTANCE m_pfnCreate;
};

//
// Standard DLL functions
//

STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hModule = (HMODULE)hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    return TRUE;
}

STDAPI DllCanUnloadNow()
{
    return (g_cRefModule == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv)
{
    return CClassFactory::CreateInstance(clsid, c_rgClassObjectInit, sizeof(c_rgClassObjectInit) / sizeof(c_rgClassObjectInit[0]), riid, ppv);
}


STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;

    // Register the bytestream handler's CLSID as a COM object.
    hr = RegisterObject(
            g_hModule,                              // Module handle
            CLSID_PpboxMediaSdk,       // CLSID
            sDescription,          // Description
            TEXT("Both")                            // Threading model
            );

    return hr;
}

STDAPI DllUnregisterServer()
{
    // Unregister the CLSIDs
    UnregisterObject(CLSID_PpboxMediaSdk);

    return S_OK;
}


///////////////////////////////////////////////////////////////////////
// Name: CreateRegistryKey
// Desc: Creates a new registry key. (Thin wrapper just to encapsulate
//       all of the default options.)
///////////////////////////////////////////////////////////////////////

HRESULT CreateRegistryKey(HKEY hKey, LPCTSTR subkey, HKEY *phKey)
{
    assert(phKey != NULL);

    LONG lreturn = RegCreateKeyEx(
        hKey,                 // parent key
        subkey,               // name of subkey
        0,                    // reserved
        NULL,                 // class string (can be NULL)
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,                 // security attributes
        phKey,
        NULL                  // receives the "disposition" (is it a new or existing key)
        );

    return HRESULT_FROM_WIN32(lreturn);
}

// Converts a CLSID into a string with the form "CLSID\{clsid}"
HRESULT CreateObjectKeyName(const GUID& guid, TCHAR *sName, DWORD cchMax)
{
    // convert CLSID uuid to string
    OLECHAR szCLSID[CHARS_IN_GUID];
    HRESULT hr = StringFromGUID2(guid, szCLSID, CHARS_IN_GUID);
    if (SUCCEEDED(hr))
    {
        // Create a string of the form "CLSID\{clsid}"
        hr = StringCchPrintf(sName, cchMax, TEXT("Software\\Classes\\CLSID\\%ls"), szCLSID);
    }
    return hr;
}

// Creates a registry key (if needed) and sets the default value of the key
HRESULT CreateRegKeyAndValue(
    HKEY hKey,
    PCWSTR pszSubKeyName,
    PCWSTR pszValueName,
    PCWSTR pszData,
    PHKEY phkResult
    )
{
    *phkResult = NULL;

    LONG lRet = RegCreateKeyEx(
        hKey, pszSubKeyName,
        0,  NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, phkResult, NULL);

    if (lRet == ERROR_SUCCESS)
    {
        lRet = RegSetValueExW(
            (*phkResult),
            pszValueName, 0, REG_SZ,
            (LPBYTE) pszData,
            ((DWORD) wcslen(pszData) + 1) * sizeof(WCHAR)
            );

        if (lRet != ERROR_SUCCESS)
        {
            RegCloseKey(*phkResult);
        }
    }

    return HRESULT_FROM_WIN32(lRet);
}

//---------------------------------------------------------------------------
//
// EliminateSubKey
//
// Try to enumerate all keys under this one.
// if we find anything, delete it completely.
// Otherwise just delete it.
//
// note - this was pinched/duplicated from
// Filgraph\Mapper.cpp - so should it be in
// a lib somewhere?
//
//---------------------------------------------------------------------------

#define MAX_KEY_LEN  260

STDAPI
RegDeleteTree( HKEY hkey, LPTSTR strSubKey )
{
    HKEY hk;
    if (0 == lstrlen(strSubKey) ) {
        // defensive approach
        return E_FAIL;
    }
  
    LONG lreturn = RegOpenKeyEx( hkey
                               , strSubKey
                               , 0
                               , MAXIMUM_ALLOWED
                               , &hk );
  
    if( ERROR_SUCCESS == lreturn )
    {
        // Keep on enumerating the first (zero-th)
        // key and deleting that
  
        for( ; ; )
        {
            TCHAR Buffer[MAX_KEY_LEN];
            DWORD dw = MAX_KEY_LEN;
            FILETIME ft;
  
            lreturn = RegEnumKeyEx( hk
                                  , 0
                                  , Buffer
                                  , &dw
                                  , NULL
                                  , NULL
                                  , NULL
                                  , &ft);
  
            if( ERROR_SUCCESS == lreturn )
            {
                RegDeleteTree(hk, Buffer);
            }
            else
            {
                break;
            }
        }
  
        RegCloseKey(hk);
        RegDeleteKey(hkey, strSubKey);
    }
  
    return NOERROR;
}


// Creates the registry entries for a COM object.

HRESULT RegisterObject(
    HMODULE hModule,
    const GUID& guid,
    const TCHAR *pszDescription,
    const TCHAR *pszThreadingModel
    )
{
    HKEY hKey = NULL;
    HKEY hSubkey = NULL;

    TCHAR achTemp[MAX_PATH];

    // Create the name of the key from the object's CLSID
    HRESULT hr = CreateObjectKeyName(guid, achTemp, MAX_PATH);

    // Create the new key.
    if (SUCCEEDED(hr))
    {
        hr = CreateRegKeyAndValue(
            HKEY_LOCAL_MACHINE,
            achTemp,
            NULL,
            pszDescription,
            &hKey
            );
    }

    if (SUCCEEDED(hr))
    {
        (void)GetModuleFileName(hModule, achTemp, MAX_PATH);

        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    // Create the "InprocServer32" subkey
    if (SUCCEEDED(hr))
    {
        hr = CreateRegKeyAndValue(
            hKey,
            L"InProcServer32",
            NULL,
            achTemp,
            &hSubkey
            );

        RegCloseKey(hSubkey);
    }

    // Add a new value to the subkey, for "ThreadingModel" = <threading model>
    if (SUCCEEDED(hr))
    {
        hr = CreateRegKeyAndValue(
            hKey,
            L"InProcServer32",
            L"ThreadingModel",
            pszThreadingModel,
            &hSubkey
            );

        RegCloseKey(hSubkey);
    }

    // close hkeys

    RegCloseKey(hKey);

    return hr;
}

// Deletes the registry entries for a COM object.

HRESULT UnregisterObject(const GUID& guid)
{
    TCHAR achTemp[MAX_PATH];

    HRESULT hr = CreateObjectKeyName(guid, achTemp, MAX_PATH);

    if (SUCCEEDED(hr))
    {
        // Delete the key recursively.
        LONG lRes = RegDeleteTree(HKEY_LOCAL_MACHINE, achTemp);

        hr = HRESULT_FROM_WIN32(lRes);
    }

    return hr;
}


