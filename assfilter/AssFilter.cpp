#include "stdafx.h"
#include "AssFilter.h"

#include "AssPin.h"
#include "SubFrame.h"

AssFilter::AssFilter(LPUNKNOWN pUnk, HRESULT* pResult)
	: CBaseFilter("", pUnk, this, __uuidof(AssFilter))
{
    if (pResult) *pResult = S_OK;
    m_pin = std::make_unique<AssPin>(this, pResult);

    m_ass = decltype(m_ass)(ass_library_init());
    m_renderer = decltype(m_renderer)(ass_renderer_init(m_ass.get()));

    m_stringOptions["name"] = L"AssFilter";
    m_stringOptions["version"] = L"0.0";
    m_stringOptions["yuvMatrix"] = L"None";
    m_boolOptions["combineBitmaps"] = false;
}

AssFilter::~AssFilter()
{
    if (m_consumer)
        m_consumer->Disconnect();
}

CUnknown* WINAPI AssFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT* pResult)
{
    try
    {
        return new AssFilter(pUnk, pResult);
    }
    catch (std::bad_alloc&)
    {
        if (pResult) *pResult = E_OUTOFMEMORY;
    }

    return nullptr;
}

void AssFilter::SetMediaType(const CMediaType& mt, IPin* pPin)
{
    CAutoLock lock(this);

    m_track = decltype(m_track)(ass_new_track(m_ass.get()));

    struct SUBTITLEINFO
    {
        DWORD dwOffset;
        CHAR  IsoLang[4];
        WCHAR TrackName[256];
    };

    auto psi = reinterpret_cast<const SUBTITLEINFO*>(mt.Format());

    ass_process_codec_private(m_track.get(), (char*)mt.Format() + psi->dwOffset, mt.FormatLength() - psi->dwOffset);

    IAMGraphStreamsPtr graphStreams;
    IDSMResourceBagPtr bag;
    if (SUCCEEDED(GetFilterGraph()->QueryInterface(IID_PPV_ARGS(&graphStreams))) &&
        SUCCEEDED(graphStreams->FindUpstreamInterface(pPin, IID_PPV_ARGS(&bag), AM_INTF_SEARCH_FILTER)))
    {
        for (DWORD i = 0; i < bag->ResGetCount(); i++)
        {
            _bstr_t name, desc, mime;
            BYTE* pData = nullptr;
            DWORD len = 0;
            if (SUCCEEDED(bag->ResGet(i, &name.GetBSTR(), &desc.GetBSTR(), &mime.GetBSTR(), &pData, &len, nullptr)))
            {
                if (wcscmp(mime.GetBSTR(), L"application/x-truetype-font") == 0 ||
                    wcscmp(mime.GetBSTR(), L"application/vnd.ms-opentype") == 0) // TODO: more mimes?
                {
                    ass_add_font(m_ass.get(), "", (char*)pData, len);
                    // TODO: clear these fonts somewhere?
                }
                CoTaskMemFree(pData);
            }
        }
    }

    ass_set_fonts(m_renderer.get(), 0, 0, ASS_FONTPROVIDER_DIRECTWRITE, 0, 0);
}

void AssFilter::Receive(IMediaSample* pSample, REFERENCE_TIME tSegmentStart)
{
    // TODO: resolve a race with RequestFrame()

    REFERENCE_TIME tStart, tStop;
    BYTE* pData;

    if (SUCCEEDED(pSample->GetTime(&tStart, &tStop)) &&
        SUCCEEDED(pSample->GetPointer(&pData)))
    {
        tStart += tSegmentStart;
        tStop += tSegmentStart;

        ass_process_chunk(m_track.get(), (char*)pData, pSample->GetSize(), tStart / 10000, (tStop - tStart) / 10000);
    }
}

STDMETHODIMP AssFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid == __uuidof(ISubRenderOptions))
        return GetInterface(static_cast<ISubRenderOptions*>(this), ppv);

    if (riid == __uuidof(ISubRenderProvider))
        return GetInterface(static_cast<ISubRenderProvider*>(this), ppv);

    return __super::NonDelegatingQueryInterface(riid, ppv);
}

int AssFilter::GetPinCount()
{
	return 1;
}

CBasePin* AssFilter::GetPin(int n)
{
	return m_pin.get();
}

STDMETHODIMP AssFilter::Pause()
{
    CAutoLock lock(this);

    if (m_pGraph)
    {
        IEnumFiltersPtr filters;
        if (SUCCEEDED(m_pGraph->EnumFilters(&filters)))
        {
            ISubRenderConsumerPtr consumer;
            ISubRenderProviderPtr provider;

            for (IBaseFilterPtr filter; filters->Next(1, &filter, 0) == S_OK;)
            {
                if (SUCCEEDED(filter->QueryInterface(IID_PPV_ARGS(&consumer))) &&
                    SUCCEEDED(QueryInterface(IID_PPV_ARGS(&provider))) &&
                    SUCCEEDED(consumer->Connect(provider)))
                {
                    m_consumer = consumer;
                    m_consumerLastId = 0;
                }
            }
        }
    }

    return __super::Pause();
}

STDMETHODIMP AssFilter::Stop()
{
    CAutoLock lock(this);

    if (m_consumer)
    {
        m_consumer->Disconnect();
        m_consumer = nullptr;
    }

    return __super::Stop();
}

STDMETHODIMP AssFilter::RequestFrame(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context)
{
    CAutoLock lock(this);

    CheckPointer(m_consumer, E_UNEXPECTED);
    RECT videoOutputRect;
    m_consumer->GetRect("videoOutputRect", &videoOutputRect);

    ass_set_frame_size(m_renderer.get(), videoOutputRect.right , videoOutputRect.bottom);

    int frameChange = 0;
    ISubRenderFramePtr frame = new SubFrame(videoOutputRect, m_consumerLastId++,
                                            ass_render_frame(m_renderer.get(), m_track.get(), start / 10000, &frameChange));
    return m_consumer->DeliverFrame(start, stop, context, frame);
}

STDMETHODIMP AssFilter::Disconnect(void)
{
    CAutoLock lock(this);
    m_consumer = nullptr;
    return S_OK;
}

STDMETHODIMP AssFilter::GetBool(LPCSTR field, bool* value)
{
    CheckPointer(value, E_POINTER);
    *value = m_boolOptions[field];
    return S_OK;
}

STDMETHODIMP AssFilter::GetInt(LPCSTR field, int* value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::GetSize(LPCSTR field, SIZE* value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::GetRect(LPCSTR field, RECT* value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::GetUlonglong(LPCSTR field, ULONGLONG* value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::GetDouble(LPCSTR field, double* value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::GetString(LPCSTR field, LPWSTR* value, int* chars)
{
    CheckPointer(value, E_POINTER);
    auto str = m_stringOptions[field];
    size_t len = str.length();
    *value = (LPWSTR)LocalAlloc(0, (len + 1) * sizeof(WCHAR));
    memcpy(*value, str.data(), len * sizeof(WCHAR));
    (*value)[len] = '\0';
    return S_OK;
}

STDMETHODIMP AssFilter::GetBin(LPCSTR field, LPVOID* value, int* size)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::SetBool(LPCSTR field, bool value)
{
    m_boolOptions[field] = value;
    return S_OK;
}

STDMETHODIMP AssFilter::SetInt(LPCSTR field, int value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::SetSize(LPCSTR field, SIZE value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::SetRect(LPCSTR field, RECT value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::SetUlonglong(LPCSTR field, ULONGLONG value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::SetDouble(LPCSTR field, double value)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::SetString(LPCSTR field, LPWSTR value, int chars)
{
    return S_FALSE;
}

STDMETHODIMP AssFilter::SetBin(LPCSTR field, LPVOID value, int size)
{
    return S_FALSE;
}

void AssFilter::ASS_LibraryDeleter::operator()(ASS_Library* p)
{
    if (p) ass_library_done(p);
}

void AssFilter::ASS_RendererDeleter::operator()(ASS_Renderer* p)
{
    if (p) ass_renderer_done(p);
}

void AssFilter::ASS_TrackDeleter::operator()(ASS_Track* p)
{
    if (p) ass_free_track(p);
}
