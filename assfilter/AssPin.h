#pragma once

class AssFilter;

class AssPin final
    : public CCritSec
    , public CBaseInputPin
{
public:

    AssPin(AssFilter* pFilter, HRESULT* pResult);

    DECLARE_IUNKNOWN;

    // CBasePin
    HRESULT CheckMediaType(const CMediaType* pmt) override;
    HRESULT SetMediaType(const CMediaType* pmt) override;

    // IMemInputPin
    STDMETHODIMP Receive(IMediaSample* pSample) override;

private:

    AssFilter* const m_pAssFilter;
};
