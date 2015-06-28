#pragma once

#include <ass.h>

class SubFrame final
    : public CUnknown
    , public ISubRenderFrame
{
public:

    SubFrame(RECT rect, ULONGLONG id, ASS_Image* image);

    DECLARE_IUNKNOWN;

    // CUnknown
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

    // ISubRenderFrame
    STDMETHODIMP GetOutputRect(RECT* outputRect) override;
    STDMETHODIMP GetClipRect(RECT* clipRect) override;
    STDMETHODIMP GetBitmapCount(int* count) override;
    STDMETHODIMP GetBitmap(int index, ULONGLONG* id, POINT* position, SIZE* size, LPCVOID* pixels, int* pitch) override;

private:

    void Flatten(ASS_Image* image);

    const RECT m_rect;
    const ULONGLONG m_id;

    std::unique_ptr<uint32_t[]> m_pixels;
    RECT m_pixelsRect;
};
