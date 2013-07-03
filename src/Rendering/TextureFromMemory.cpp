#include "stdafx.h"

#include "TextureFromMemory.h"

#include "RenderDevice.h"
#include <Core/Cast.h>
#include <Core/FilePath.h>
#include <Core/IntTypes.h>
#include <Core/Log.h>
#include <Core/StringFormat.h>
#include <d3d11.h>

// TODO:
// - Allow mipmap generation ?

namespace sg {
namespace rendering {
//=============================================================================
TextureFromMemory::TextureFromMemory(RenderDevice const* iRenderDevice)
    : m_renderDevice(iRenderDevice)
    , m_texture()
    , m_shaderResourceView()
    , m_resolution()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextureFromMemory::~TextureFromMemory()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextureFromMemory::ClearAndSetResolution(uint2 const& iResolution)
{
    m_shaderResourceView = nullptr;
    m_texture = nullptr;
    m_resolution.Set(iResolution);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextureFromMemory::UpdateFromMemory(u8* iData, size_t iDataSize, ColorFormat iColorFormat, uint2 const& iResolution, size_t iStrideInBytes)
{
    m_resolution.Set(iResolution);
    UpdateAssumeSameResolution(iData, iDataSize, iColorFormat, iResolution, iStrideInBytes);

}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextureFromMemory::UpdateAssumeSameResolution(u8* iData, size_t iDataSize, ColorFormat iColorFormat, uint2 const& iResolution, size_t iStrideInBytes)
{
    ID3D11Device* device = m_renderDevice->D3DDevice();

    uint2 const res = m_resolution.Get();
    SG_ASSERT_AND_UNUSED(iResolution == res);
    size_t const area = res.y() * res.x();
    SG_ASSERT(0 != area);
    if(0 == area)
        return;

    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    switch(iColorFormat)
    {
    case ColorFormat::R8: format = DXGI_FORMAT_R8_UNORM; break;
    case ColorFormat::R8G8: format = DXGI_FORMAT_R8G8_UNORM; break;
    case ColorFormat::R8G8B8A8: format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
    case ColorFormat::R8G8B8A8_SRGB: format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
    default:
        SG_ASSERT_NOT_REACHED();
    }

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = res.x();
    desc.Height = res.y();
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_IMMUTABLE ;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    SG_ASSERT_AND_UNUSED(iDataSize >= (res.y()-1) * iStrideInBytes + res.x() * SizeOf(iColorFormat));

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = iData;
    subresourceData.SysMemPitch = UINT(iStrideInBytes);
    subresourceData.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateTexture2D(
        &desc,
        &subresourceData,
        m_texture.GetPointerForInitialisation()
        );
    SG_ASSERT(SUCCEEDED(hr));

#if SG_ENABLE_ASSERT
    rendering::SetDebugName(m_texture.get(), Format("TextureFromMemory (%0 x %1)", res.x(), res.y()));
#endif

    hr = device->CreateShaderResourceView(
        m_texture.get(),
        NULL,
        m_shaderResourceView.GetPointerForInitialisation()
        );
    SG_ASSERT(SUCCEEDED(hr));
}
//=============================================================================
TextureFromOwnMemory::TextureFromOwnMemory(RenderDevice const* iRenderDevice, ColorFormat iColorFormat, uint2 const& iResolution)
    : m_impl(iRenderDevice)
    , m_data()
    , m_colorFormat()
    , m_isUpToDate(false)
{
    Reset(iColorFormat, iResolution);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextureFromOwnMemory::~TextureFromOwnMemory()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextureFromOwnMemory::Reset(ColorFormat iColorFormat, uint2 const& iResolution)
{
    m_impl.ClearAndSetResolution(iResolution);
    m_isUpToDate = false;
    m_data.clear();
    m_colorFormat = iColorFormat;
    uint2 const res = m_impl.Resolution();
    size_t const area = res.y() * res.x();
    if(0 != area)
        m_data.resize(area * TextureFromMemory::SizeOf(m_colorFormat), 0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextureFromOwnMemory::Update()
{
    SG_ASSERT(!m_isUpToDate);

    uint2 const res = m_impl.Resolution();
    size_t const area = res.y() * res.x();
    SG_ASSERT(0 != area);
    if(0 == area)
        return;

    m_impl.UpdateAssumeSameResolution(m_data.data(), m_data.size(), m_colorFormat, res, res.x() * TextureFromMemory::SizeOf(m_colorFormat));

    m_isUpToDate = true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextureFromOwnMemory::ForgetMemory()
{
    SG_ASSERT(m_isUpToDate);
    std::vector<u8>().swap(m_data);
    SG_ASSERT(0 == m_data.capacity());
}
//=============================================================================
}
}

