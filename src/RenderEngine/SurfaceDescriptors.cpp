#include "stdafx.h"

#include "SurfaceDescriptors.h"

#include <Rendering/Surface.h>
#include <Rendering/TextureFromFile.h>
#include "Compositing.h"
#include "ResolutionDescriptors.h"

namespace sg {
namespace renderengine {
//=============================================================================
AbstractSurfaceDescriptor::AbstractSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
AbstractSurfaceDescriptor::~AbstractSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_ABSTRACT_CLASS_BEGIN((sg, renderengine), AbstractSurfaceDescriptor)
REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
InputSurfaceDescriptor::InputSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
InputSurfaceDescriptor::~InputSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IRenderTarget const* InputSurfaceDescriptor::GetAsRenderTarget(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_MSG(false,"Can not be used as render target");
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResource const* InputSurfaceDescriptor::GetAsShaderResource(Compositing* iCompositing) const
{
    return iCompositing->GetShaderResource(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::BaseSurface* InputSurfaceDescriptor::CreateSurface(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_MSG(false,"Can not create such surface");
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), InputSurfaceDescriptor)
REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
OutputSurfaceDescriptor::OutputSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
OutputSurfaceDescriptor::~OutputSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IRenderTarget const* OutputSurfaceDescriptor::GetAsRenderTarget(Compositing* iCompositing) const
{
    return iCompositing->GetRenderTarget(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResource const* OutputSurfaceDescriptor::GetAsShaderResource(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_MSG(false,"can not be used as shader resource");
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::BaseSurface* OutputSurfaceDescriptor::CreateSurface(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_MSG(false,"Can not create such surface");
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), OutputSurfaceDescriptor)
REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
SurfaceDescriptor::SurfaceDescriptor()
: m_mipLevels(0)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SurfaceDescriptor::~SurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IRenderTarget const* SurfaceDescriptor::GetAsRenderTarget(Compositing* iCompositing) const
{
    return iCompositing->GetSurface(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResource const* SurfaceDescriptor::GetAsShaderResource(Compositing* iCompositing) const
{
    return iCompositing->GetSurface(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::Surface const* SurfaceDescriptor::GetAsSurface(Compositing* iCompositing) const
{
    return iCompositing->GetSurface(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::Surface* SurfaceDescriptor::CreateSurface(Compositing* iCompositing) const
{
    rendering::SurfaceProperties properties;
    properties.mipLevels = m_mipLevels;
    properties.baseFormat = DXGI_FORMAT_B8G8R8A8_TYPELESS;
    properties.readFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    properties.writeFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    rendering::ResolutionServer const* resolutionServer = iCompositing->GetResolutionServer(m_resolution.get());
    SG_ASSERT(nullptr != resolutionServer);
    rendering::Surface* surface = new rendering::Surface(iCompositing->RenderDevice(), resolutionServer, &properties);
    return surface;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void SurfaceDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    REFLECTION_CHECK_PROPERTY_NotNull(resolution);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), SurfaceDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(resolution, "")
    REFLECTION_m_PROPERTY_DOC(mipLevels, "")
REFLECTION_CLASS_END
//=============================================================================
DepthStencilSurfaceDescriptor::DepthStencilSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
DepthStencilSurfaceDescriptor::~DepthStencilSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IRenderTarget const* DepthStencilSurfaceDescriptor::GetAsRenderTarget(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_NOT_REACHED();
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResource const* DepthStencilSurfaceDescriptor::GetAsShaderResource(Compositing* iCompositing) const
{
    return iCompositing->GetDepthStencilSurface(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::DepthStencilSurface const* DepthStencilSurfaceDescriptor::GetAsSurface(Compositing* iCompositing) const
{
    return iCompositing->GetDepthStencilSurface(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::DepthStencilSurface* DepthStencilSurfaceDescriptor::CreateSurface(Compositing* iCompositing) const
{
    rendering::SurfaceProperties properties;
    properties.mipLevels = 1;
    properties.baseFormat = DXGI_FORMAT_R16_TYPELESS;
    properties.readFormat = DXGI_FORMAT_R16_UNORM;
    properties.writeFormat = DXGI_FORMAT_D16_UNORM;
    rendering::ResolutionServer const* resolutionServer = iCompositing->GetResolutionServer(m_resolution.get());
    SG_ASSERT(nullptr != resolutionServer);
    rendering::DepthStencilSurface* surface = new rendering::DepthStencilSurface(iCompositing->RenderDevice(), resolutionServer, &properties);
    return surface;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void DepthStencilSurfaceDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    REFLECTION_CHECK_PROPERTY_NotNull(resolution);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), DepthStencilSurfaceDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(resolution, "")
    REFLECTION_m_PROPERTY_DOC(mipLevels, "")
REFLECTION_CLASS_END
//=============================================================================
TextureSurfaceDescriptor::TextureSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextureSurfaceDescriptor::~TextureSurfaceDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IRenderTarget const* TextureSurfaceDescriptor::GetAsRenderTarget(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_MSG(false,"can not be used as render target");
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::IShaderResource const* TextureSurfaceDescriptor::GetAsShaderResource(Compositing* iCompositing) const
{
    return iCompositing->GetShaderResource(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::BaseSurface* TextureSurfaceDescriptor::CreateSurface(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_MSG(false,"This method should not be called");
    return nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
rendering::TextureFromFile const* TextureSurfaceDescriptor::CreateTexture(Compositing* iCompositing) const
{
    rendering::RenderDevice const* renderDevice = iCompositing->RenderDevice();
    rendering::TextureFromFile* texture = new rendering::TextureFromFile(renderDevice, m_filename);
    // Here, we assume that client will take a reference on that texture. If not, there will be a leak.
    return texture;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), TextureSurfaceDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(filename, "")
REFLECTION_CLASS_END
//=============================================================================
}
}

