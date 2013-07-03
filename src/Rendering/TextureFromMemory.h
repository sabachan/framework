#ifndef Rendering_TextureOnMemory_H
#define Rendering_TextureOnMemory_H

#include <Core/ComPtr.h>
#include <Core/SmartPtr.h>
#include <Image/Image.h>
#include "ResolutionServer.h"
#include "ShaderResource.h"

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace sg {
namespace rendering {
//=============================================================================
class RenderDevice;
//=============================================================================
class TextureFromMemory : public RefAndSafeCountable
                        , public IShaderResource
{
    PARENT_REF_COUNTABLE(RefAndSafeCountable)
    PARENT_SAFE_COUNTABLE(RefAndSafeCountable)
public:
    enum class ColorFormat { R8, R8G8, R8G8B8A8, R8G8B8A8_SRGB };
    static size_t SizeOf(ColorFormat iColorFormat);

    TextureFromMemory(RenderDevice const* iRenderDevice);
    virtual ~TextureFromMemory() override;

    void UpdateFromMemory(u8* iData, size_t iDataSize, ColorFormat iColorFormat, uint2 const& iResolution, size_t iStrideInBytes);

    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { SG_ASSERT(nullptr != m_shaderResourceView); return m_shaderResourceView.get(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { return &m_resolution; }
private:
    friend class TextureFromOwnMemory;
    void ClearAndSetResolution(uint2 const& iResolution);
    void UpdateAssumeSameResolution(u8* iData, size_t iDataSize, ColorFormat iColorFormat, uint2 const& iResolution, size_t iStrideInBytes);
    inline uint2 const& Resolution() const { return m_resolution.Get(); }
private:
    safeptr<RenderDevice const> m_renderDevice;
    comptr<ID3D11Texture2D> m_texture;
    comptr<ID3D11ShaderResourceView> m_shaderResourceView;
    ResolutionServer m_resolution;
};
//=============================================================================
inline size_t TextureFromMemory::SizeOf(ColorFormat iColorFormat)
{
    size_t const sizes[] = { 1, 2, 4, 4, };
    size_t const i = static_cast<size_t>(iColorFormat);
    SG_ASSERT(i < SG_ARRAYSIZE(sizes));
    return sizes[i];
}
//=============================================================================
class TextureFromOwnMemory : public RefAndSafeCountable
                           , public IShaderResource
{
    PARENT_REF_COUNTABLE(RefAndSafeCountable)
    PARENT_SAFE_COUNTABLE(RefAndSafeCountable)
public:
    typedef TextureFromMemory::ColorFormat ColorFormat;
    TextureFromOwnMemory(RenderDevice const* iRenderDevice, ColorFormat iColorFormat = ColorFormat::R8, uint2 const& iResolution = uint2(0,0));
    virtual ~TextureFromOwnMemory() override;

    template<typename T> void UpdateFromExtMemory(u8* iData, size_t iDataSize, ColorFormat iColorFormat, uint2 const& iResolution, size_t iStrideInBytes);
    void Reset(ColorFormat iColorFormat, uint2 const& iResolution);
    void Reset(uint2 const& iResolution) { Reset(m_colorFormat, iResolution); }
    template<typename T> image::ImageView<T> GetAsImageForModification();
    void UpdateIFN() { if(!m_isUpToDate) Update(); }
    void Update_AssumeModified() { SG_ASSERT(!m_isUpToDate); Update(); }
    void ForgetMemory();

    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { SG_ASSERT(m_isUpToDate); return m_impl.TextureFromMemory::GetShaderResourceView(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { return m_impl.TextureFromMemory::ShaderResourceResolution(); }
private:
    void Update();
private:
    TextureFromMemory m_impl;
    std::vector<u8> m_data;
    ColorFormat m_colorFormat;
    bool m_isUpToDate;
};
//=============================================================================
template<typename T>
inline image::ImageView<T> TextureFromOwnMemory::GetAsImageForModification()
{
    SG_ASSERT(!m_data.empty());
    SG_ASSERT(TextureFromMemory::SizeOf(m_colorFormat) == sizeof(T));
    uint2 const res = m_impl.Resolution();
    SG_ASSERT(m_data.size() == res.x() * res.y() * TextureFromMemory::SizeOf(m_colorFormat));
    m_isUpToDate = false;
    return image::ImageView<T>(m_data.data(), res, res.x() * sizeof(T));
}
//=============================================================================
}
}

#endif
