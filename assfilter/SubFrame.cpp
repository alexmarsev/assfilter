#include "stdafx.h"
#include "SubFrame.h"

namespace
{
    inline POINT GetRectPos(RECT rect)
    {
        return {rect.left, rect.top};
    }

    inline SIZE GetRectSize(RECT rect)
    {
        return {rect.right - rect.left, rect.bottom - rect.top};
    }
}

SubFrame::SubFrame(RECT rect, ULONGLONG id, ASS_Image* image)
    : CUnknown("", nullptr)
    , m_rect(rect)
    , m_id(id)
{
    Flatten(image);
}

STDMETHODIMP SubFrame::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid == __uuidof(ISubRenderFrame))
        return GetInterface(static_cast<ISubRenderFrame*>(this), ppv);

    return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP SubFrame::GetOutputRect(RECT* outputRect)
{
    CheckPointer(outputRect, E_POINTER);
    *outputRect = m_rect;
    return S_OK;
}

STDMETHODIMP SubFrame::GetClipRect(RECT* clipRect)
{
    CheckPointer(clipRect, E_POINTER);
    *clipRect = m_rect;
    return S_OK;
}

STDMETHODIMP SubFrame::GetBitmapCount(int* count)
{
    CheckPointer(count, E_POINTER);
    *count = (m_pixels ? 1 : 0);
    return S_OK;
}

STDMETHODIMP SubFrame::GetBitmap(int index, ULONGLONG* id, POINT* position, SIZE* size, LPCVOID* pixels, int* pitch)
{
    if (index != 0) return E_INVALIDARG;
    CheckPointer(id, E_POINTER);
    CheckPointer(position, E_POINTER);
    CheckPointer(size, E_POINTER);
    CheckPointer(pixels, E_POINTER);
    CheckPointer(pitch, E_POINTER);

    *id = m_id;
    *position = GetRectPos(m_pixelsRect);
    *size = GetRectSize(m_pixelsRect);
    *pixels = m_pixels.get();
    *pitch = size->cx * 4;

    return S_OK;
}

void SubFrame::Flatten(ASS_Image* image)
{
    if (image)
    {
        for (auto i = image; i != nullptr; i = i->next)
        {
            RECT rect1 = m_pixelsRect;
            RECT rect2 = {i->dst_x, i->dst_y, i->dst_x + i->w, i->dst_y + i->h};
            UnionRect(&m_pixelsRect, &rect1, &rect2);
        }

        const POINT pixelsPoint = GetRectPos(m_pixelsRect);
        const SIZE pixelsSize = GetRectSize(m_pixelsRect);
        m_pixels = std::make_unique<uint32_t[]>(pixelsSize.cx * pixelsSize.cy);

        for (auto i = image; i != nullptr; i = i->next)
        {
            for (int y = 0; y < i->h; y++)
            {
                for (int x = 0; x < i->w; x++)
                {
                    uint32_t& dest = m_pixels[(i->dst_y + y - pixelsPoint.y) * pixelsSize.cx +
                                              (i->dst_x + x - pixelsPoint.x)];

                    uint32_t destA = (dest & 0xff000000) >> 24;

                    uint32_t srcA = i->bitmap[y * i->stride + x] * (0xff - (i->color & 0x000000ff));
                    srcA >>= 8;

                    uint32_t outA = srcA + ((destA * (0xff - srcA)) >> 8);

                    uint32_t outR = ((i->color & 0xff000000) >> 8) * srcA + (dest & 0x00ff0000) * (0xff - srcA);
                    outR >>= 8;

                    uint32_t outG = ((i->color & 0x00ff0000) >> 8) * srcA + (dest & 0x0000ff00) * (0xff - srcA);
                    outG >>= 8;

                    uint32_t outB = ((i->color & 0x0000ff00) >> 8) * srcA + (dest & 0x000000ff) * (0xff - srcA);
                    outB >>= 8;

                    dest = (outA << 24) + (outR & 0x00ff0000) + (outG & 0x0000ff00) + (outB & 0x000000ff);
                }
            }
        }
    }
}
