#pragma once

#include <initguid.h>

#include <comdef.h>

#include <streams.h>

#include <cassert>
#include <map>
#include <memory>
#include <string>

#include "SubRenderIntf.h"

_COM_SMARTPTR_TYPEDEF(IEnumFilters, __uuidof(IEnumFilters));
_COM_SMARTPTR_TYPEDEF(IBaseFilter, __uuidof(IBaseFilter));
_COM_SMARTPTR_TYPEDEF(ISubRenderProvider, __uuidof(ISubRenderProvider));
_COM_SMARTPTR_TYPEDEF(ISubRenderConsumer, __uuidof(ISubRenderConsumer));
_COM_SMARTPTR_TYPEDEF(ISubRenderFrame, __uuidof(ISubRenderFrame));
_COM_SMARTPTR_TYPEDEF(IAMGraphStreams, __uuidof(IAMGraphStreams));

// {00000000-0000-0000-0000-000000000000}
DEFINE_GUID(GUID_NULL, 0x00000000, 0x6000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

// {E487EB08-6B26-4be9-9DD3-993434D313FD}
DEFINE_GUID(MEDIATYPE_Subtitle, 0xe487eb08, 0x6b26, 0x4be9, 0x9d, 0xd3, 0x99, 0x34, 0x34, 0xd3, 0x13, 0xfd);

// {326444F7-686F-47ff-A4B2-C8C96307B4C2}
DEFINE_GUID(MEDIASUBTYPE_ASS, 0x326444f7, 0x686f, 0x47ff, 0xa4, 0xb2, 0xc8, 0xc9, 0x63, 0x7, 0xb4, 0xc2);

// {A33D2F7D-96BC-4337-B23B-A8B9FBC295E9}
DEFINE_GUID(FORMAT_SubtitleInfo, 0xa33d2f7d, 0x96bc, 0x4337, 0xb2, 0x3b, 0xa8, 0xb9, 0xfb, 0xc2, 0x95, 0xe9);

#define ReturnIfFailed(x) { HRESULT hr = (x); if (FAILED(hr)) return hr; }
#define ReturnIfNotEquals(r, x) { HRESULT hr = (x); if (hr != r) return hr; }

interface __declspec(uuid("EBAFBCBE-BDE0-489A-9789-05D5692E3A93"))
IDSMResourceBag : public IUnknown {
    STDMETHOD_(DWORD, ResGetCount)() PURE;
    STDMETHOD(ResGet)(DWORD iIndex, BSTR* ppName, BSTR* ppDesc, BSTR* ppMime, BYTE** ppData, DWORD* pDataLen, DWORD_PTR* pTag) PURE;
    STDMETHOD(ResSet)(DWORD iIndex, LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime, const BYTE* pData, DWORD len, DWORD_PTR tag) PURE;
    STDMETHOD(ResAppend)(LPCWSTR pName, LPCWSTR pDesc, LPCWSTR pMime, BYTE* pData, DWORD len, DWORD_PTR tag) PURE;
    STDMETHOD(ResRemoveAt)(DWORD iIndex) PURE;
    STDMETHOD(ResRemoveAll)(DWORD_PTR tag) PURE;
};
_COM_SMARTPTR_TYPEDEF(IDSMResourceBag, __uuidof(IDSMResourceBag));
