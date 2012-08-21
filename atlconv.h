// atlconv.h

#include <wtypes.h>
#include <assert.h>
#include <malloc.h>

#define ASSERT(f) assert(f)
#define VERIFY(f) ASSERT(f)

#define USES_CONVERSION int _convert = 0; \
	 (void)_convert; \
	 UINT _acp = GetACP(); \
	 (void)_acp; \
	 LPCWSTR _lpw = NULL; \
	 (void)_lpw; \
	 LPCSTR _lpa = NULL; \
	 (void)_lpa


#define T2COLE(lpa) A2CW(lpa)
#define T2OLE(lpa) A2W(lpa)
#define OLE2CT(lpo) W2CA(lpo)
#define OLE2T(lpo) W2A(lpo)

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCSTR)W2A(lpw))

#ifndef ATLA2WHELPER
	#define ATLA2WHELPER AtlA2WHelper
	#define ATLW2AHELPER AtlW2AHelper
#endif

#define A2W(lpa) ( \
	((_lpa = lpa) == NULL) ? NULL : ( \
		_convert = (lstrlenA(_lpa)+1), \
		ATLA2WHELPER((LPWSTR) alloca(_convert*2), _lpa, _convert)))

#define W2A(lpw) ( \
	((_lpw = lpw) == NULL) ? NULL : ( \
		_convert = (lstrlenW(_lpw)+1)*2, \
		ATLW2AHELPER((LPSTR) alloca(_convert), _lpw, _convert)))

LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	if (lpw == NULL)
		return NULL;
	ASSERT(lpa != NULL);
	// verify that no illegal character present
	// since lpa was allocated based on the size of lpw
	// don''t worry about the number of chars
	lpa[0] = '\0';
	VERIFY(WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL));
	return lpa;
}


LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	if (lpa == NULL)
		return NULL;
	ASSERT(lpw != NULL);
	// verify that no illegal character present
	// since lpw was allocated based on the size of lpa
	// don''t worry about the number of chars
	lpw[0] = '\0';
	VERIFY(MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars));
	return lpw;
}

