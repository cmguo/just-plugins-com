// MediaSdk.h

#include "IMediaSdk.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

void DllAddRef();
void DllRelease();

class MediaSdk : public IMediaSdk
{
public:
    static HRESULT CreateInstance(REFIID iid, void **ppMEG);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    virtual HRESULT STDMETHODCALLTYPE StartP2PEngine( 
        /* [in] */ LPCOLESTR pszGid,
        /* [in] */ LPCOLESTR pszPid,
        /* [in] */ LPCOLESTR pszAuth);
    
    virtual HRESULT STDMETHODCALLTYPE StopP2PEngine();

    virtual HRESULT STDMETHODCALLTYPE DownloadOpen(
        /* [in] */ LPCOLESTR pszUrl,
        /* [in] */ LPCOLESTR pszFormat,
        /* [in] */ LPCOLESTR psz, 
        /* [out] */ LPDWORD lpdwHandle);

    virtual HRESULT STDMETHODCALLTYPE DownloadClose(
        /* [in] */ DWORD dwHandle);

    virtual HRESULT STDMETHODCALLTYPE GetDownloadInfo(
        /* [in] */ DWORD dwHandle, 
        /* [out] */ LPDownloadStatistic lpStatistic);

private:
    MediaSdk(HRESULT& hr);

    ~MediaSdk();

    long        m_cRef;     // Reference count.
};

inline HRESULT MediaSdk_CreateInstance(REFIID riid, void **ppv)
{
    return MediaSdk::CreateInstance(riid, ppv);
}

