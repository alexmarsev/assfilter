#include "stdafx.h"
#include "AssFilter.h"

namespace
{
    const WCHAR name[] = L"AssFilter";

    const AMOVIESETUP_MEDIATYPE pinTypes[] = {
        { &MEDIATYPE_Subtitle, &MEDIASUBTYPE_ASS },
    };

    const AMOVIESETUP_PIN setupPin = {
        L"", TRUE, FALSE, TRUE, TRUE, &CLSID_NULL, nullptr, _countof(pinTypes), pinTypes
    };

    const AMOVIESETUP_FILTER setupFilter = {
        &__uuidof(AssFilter), name, MERIT_UNLIKELY, 1, &setupPin, &CLSID_LegacyAmFilterCategory
    };
}

CFactoryTemplate g_Templates[] = {
    { name, &__uuidof(AssFilter), AssFilter::CreateInstance, nullptr, &setupFilter },
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
	return DllEntryPoint(hDllHandle, dwReason, lpReserved);
}
