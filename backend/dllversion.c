/* Sed: http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/versions.asp */
#if defined (_WIN32) && (defined(_USRDLL) || defined(DLL_EXPORT) || defined(PIC))
#include <windows.h>
#include <shlwapi.h>
#include "zintconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

__declspec(dllexport) HRESULT DllGetVersion (DLLVERSIONINFO2* pdvi);

#ifdef __cplusplus
}
#endif

HRESULT DllGetVersion (DLLVERSIONINFO2* pdvi)
{
	if (!pdvi || (sizeof(*pdvi) != pdvi->info1.cbSize))
		return (E_INVALIDARG);

	pdvi->info1.dwMajorVersion = ZINT_VERSION_MAJOR;
	pdvi->info1.dwMinorVersion = ZINT_VERSION_MINOR;
	pdvi->info1.dwBuildNumber = ZINT_VERSION_RELEASE;
	pdvi->info1.dwPlatformID = DLLVER_PLATFORM_WINDOWS;
	if (sizeof(DLLVERSIONINFO2) == pdvi->info1.cbSize)
		pdvi->ullVersion = MAKEDLLVERULL(ZINT_VERSION_MAJOR, ZINT_VERSION_MINOR, ZINT_VERSION_RELEASE, ZINT_VERSION_BUILD);

	return S_OK;
}
#endif /* _WIN32 */
