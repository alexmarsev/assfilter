#include "stdafx.h"
#include "AssPin.h"

#include "AssFilter.h"

AssPin::AssPin(AssFilter* pFilter, HRESULT* pResult)
    : CBaseInputPin("", pFilter, this, pResult, L"Input0")
    , m_pAssFilter(pFilter)
{
}

HRESULT AssPin::CheckMediaType(const CMediaType* pmt)
{
    CheckPointer(pmt, E_POINTER);

    if (pmt->majortype == MEDIATYPE_Subtitle &&
        pmt->subtype == MEDIASUBTYPE_ASS &&
        pmt->formattype == FORMAT_SubtitleInfo)
    {
        return S_OK;
    }

    return S_FALSE;
}

HRESULT AssPin::SetMediaType(const CMediaType* pmt)
{
    assert(CritCheckIn(this));

    ReturnIfFailed(CBaseInputPin::SetMediaType(pmt));

    m_pAssFilter->SetMediaType(*pmt, m_Connected);

    return S_OK;
}

STDMETHODIMP AssPin::Receive(IMediaSample* pSample)
{
    ReturnIfNotEquals(S_OK, CBaseInputPin::Receive(pSample));

    m_pAssFilter->Receive(pSample, m_tStart);

    return S_OK;
}
