// MediaSdk.h

#include "rpc.h"
#include "rpcndr.h"

typedef struct __DownloadStatistic
{
    DWORD dwTotalSize;
    DWORD dwFinishSize;
    DWORD dwSpeed;
} DownloadStatistic, * LPDownloadStatistic;

EXTERN_C const IID IID_IMediaSdk;

MIDL_INTERFACE("111649F0-CD79-426c-B850-514D7EE43AE2")
IMediaSdk : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE StartP2PEngine( 
        /* [in] */ LPCOLESTR pszGid,
        /* [in] */ LPCOLESTR pszPid,
        /* [in] */ LPCOLESTR pszAuth) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE StopP2PEngine() = 0;

    virtual HRESULT STDMETHODCALLTYPE DownloadOpen(
        /* [in] */ LPCOLESTR pszUrl,
        /* [in] */ LPCOLESTR pszFormat,
        /* [in] */ LPCOLESTR psz, 
        /* [out] */ LPDWORD lpdwHandle) = 0;

    virtual HRESULT STDMETHODCALLTYPE DownloadClose(
        /* [in] */ DWORD dwHandle) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetDownloadInfo(
        /* [in] */ DWORD dwHandle, 
        /* [out] */ LPDownloadStatistic lpStatistic) = 0;
    
};

