// MediaSdk.cpp

#include <new>
#include <atlconv.h>

#include "MediaSdk.h"

#define JUST_DISABLE_AUTO_START
#include <plugins/just/IPpboxRuntime.h>

//-------------------------------------------------------------------
// CreateInstance
// Static method to create an instance of the oject.
//
// This method is used by the class factory.
//
//-------------------------------------------------------------------

HRESULT MediaSdk::CreateInstance(REFIID iid, void **ppv)
{
    if (ppv == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    MediaSdk *pHandler = new (std::nothrow) MediaSdk(hr);
    if (pHandler == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = pHandler->QueryInterface(iid, ppv);
    }

    SafeRelease(&pHandler);
    return hr;
}


//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------

MediaSdk::MediaSdk(HRESULT& hr)
    : m_cRef(1)
{
    DllAddRef();
}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------

MediaSdk::~MediaSdk()
{
    DllRelease();
}


//-------------------------------------------------------------------
// IUnknown methods
//-------------------------------------------------------------------

ULONG MediaSdk::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG MediaSdk::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

HRESULT MediaSdk::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IMediaSdk) {
        AddRef();
        *ppv = this;
    };
    return E_FAIL;
}


HRESULT STDMETHODCALLTYPE MediaSdk::StartEngine( 
    /* [in] */ LPCOLESTR pszGid,
    /* [in] */ LPCOLESTR pszPid,
    /* [in] */ LPCOLESTR pszAuth)
{
    USES_CONVERSION;

    LPCSTR gid = W2A(pszGid);
    LPCSTR pid = W2A(pszPid);
    LPCSTR auth = W2A(pszAuth);

    long r = JUST_StartEngine()(gid, pid, auth);

    return r == just_success ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE MediaSdk::StopEngine()
{
    JUST_StopEngine()();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MediaSdk::DownloadOpen(
    /* [in] */ LPCOLESTR pszUrl,
    /* [in] */ LPCOLESTR pszFormat,
    /* [in] */ LPCOLESTR psz, 
    /* [out] */ LPDWORD lpdwHandle)
{
    USES_CONVERSION;

    LPCSTR url = W2A(pszUrl);
    LPCSTR format = W2A(pszFormat);
    LPCSTR file = W2A(psz);

    JUST_Download_Handle h = JUST_DownloadOpen()(url, format, file, NULL);

    if (h == JUST_INVALID_DOWNLOAD_HANDLE)
        return E_FAIL;

    *lpdwHandle = (DWORD)h;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE MediaSdk::DownloadClose(
    /* [in] */ DWORD dwHandle)
{
    JUST_Download_Handle h = (JUST_Download_Handle)dwHandle;
    JUST_DownloadClose()(h);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE MediaSdk::GetDownloadInfo(
    /* [in] */ DWORD dwHandle, 
    /* [out] */ LPDownloadStatistic lpStatistic)
{
    JUST_Download_Handle h = (JUST_Download_Handle)dwHandle;
    JUST_DownloadStatistic stat;
    long r = JUST_GetDownloadInfo()(h, &stat);
    if (r != just_success)
        return E_FAIL;
    lpStatistic->dwTotalSize = stat.total_size;
    lpStatistic->dwFinishSize = stat.finish_size;
    lpStatistic->dwSpeed = stat.speed;
    return S_OK;
}

