#pragma once

#include <ass.h>

class AssPin;

class __declspec(uuid("8A3704F3-BE3B-4944-9FF3-EE8757FDBDA5"))
AssFilter final
	: public CCritSec
	, public CBaseFilter
    , public ISubRenderProvider
{
public:

	AssFilter(LPUNKNOWN pUnk, HRESULT* pResult);
	~AssFilter();

	DECLARE_IUNKNOWN;

    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* pResult);

    void SetMediaType(const CMediaType& mt, IPin* pPin);
    void Receive(IMediaSample* pSample, REFERENCE_TIME tSegmentStart);

    // CUnknown
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

	// CBaseFilter
	int GetPinCount() override;
	CBasePin* GetPin(int n) override;
    STDMETHODIMP Pause() override;
    STDMETHODIMP Stop() override;

    // ISubRenderProvider
    STDMETHODIMP RequestFrame(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context) override;
    STDMETHODIMP Disconnect() override;

    STDMETHODIMP GetBool(LPCSTR field, bool* value) override;
    STDMETHODIMP GetInt(LPCSTR field, int* value) override;
    STDMETHODIMP GetSize(LPCSTR field, SIZE* value) override;
    STDMETHODIMP GetRect(LPCSTR field, RECT* value) override;
    STDMETHODIMP GetUlonglong(LPCSTR field, ULONGLONG* value) override;
    STDMETHODIMP GetDouble(LPCSTR field, double* value) override;
    STDMETHODIMP GetString(LPCSTR field, LPWSTR* value, int* chars) override;
    STDMETHODIMP GetBin(LPCSTR field, LPVOID* value, int* size) override;
    STDMETHODIMP SetBool(LPCSTR field, bool value) override;
    STDMETHODIMP SetInt(LPCSTR field, int value) override;
    STDMETHODIMP SetSize(LPCSTR field, SIZE value) override;
    STDMETHODIMP SetRect(LPCSTR field, RECT value) override;
    STDMETHODIMP SetUlonglong(LPCSTR field, ULONGLONG value) override;
    STDMETHODIMP SetDouble(LPCSTR field, double value) override;
    STDMETHODIMP SetString(LPCSTR field, LPWSTR value, int chars) override;
    STDMETHODIMP SetBin(LPCSTR field, LPVOID value, int size) override;

private:

    struct ASS_LibraryDeleter
    {
        void operator()(ASS_Library* p);
    };

    struct ASS_RendererDeleter
    {
        void operator()(ASS_Renderer* p);
    };

    struct ASS_TrackDeleter
    {
        void operator()(ASS_Track* p);
    };

    std::unique_ptr<ASS_Library, ASS_LibraryDeleter> m_ass;
    std::unique_ptr<ASS_Renderer, ASS_RendererDeleter> m_renderer;
    std::unique_ptr<ASS_Track, ASS_TrackDeleter> m_track;

    std::unique_ptr<AssPin> m_pin;
    ISubRenderConsumerPtr m_consumer;
    ULONGLONG m_consumerLastId = 0;
    std::map<std::string, std::wstring> m_stringOptions;
    std::map<std::string, bool> m_boolOptions;
};
