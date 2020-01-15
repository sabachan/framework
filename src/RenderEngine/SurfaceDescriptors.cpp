#include "stdafx.h"

#include "SurfaceDescriptors.h"

#include <Rendering/Surface.h>
#include <Rendering/TextureFromFile.h>
#include "Compositing.h"
#include "ResolutionDescriptors.h"

namespace sg {
namespace rendering {
//=============================================================================
REFLECTION_ENUM_BEGIN(SurfaceFormat)
    REFLECTION_ENUM_DOC("surface format")
    REFLECTION_ENUM_VALUE_DOC(UNKNOWN                     , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32B32A32_TYPELESS       , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32B32A32_FLOAT          , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32B32A32_UINT           , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32B32A32_SINT           , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32B32_TYPELESS          , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32B32_FLOAT             , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32B32_UINT              , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32B32_SINT              , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16B16A16_TYPELESS       , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16B16A16_FLOAT          , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16B16A16_UNORM          , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16B16A16_UINT           , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16B16A16_SNORM          , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16B16A16_SINT           , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32_TYPELESS             , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32_FLOAT                , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32_UINT                 , "")
    REFLECTION_ENUM_VALUE_DOC(R32G32_SINT                 , "")
    REFLECTION_ENUM_VALUE_DOC(R32G8X24_TYPELESS           , "")
    REFLECTION_ENUM_VALUE_DOC(D32_FLOAT_S8X24_UINT        , "")
    REFLECTION_ENUM_VALUE_DOC(R32_FLOAT_X8X24_TYPELESS    , "")
    REFLECTION_ENUM_VALUE_DOC(X32_TYPELESS_G8X24_UINT     , "")
    REFLECTION_ENUM_VALUE_DOC(R10G10B10A2_TYPELESS        , "")
    REFLECTION_ENUM_VALUE_DOC(R10G10B10A2_UNORM           , "")
    REFLECTION_ENUM_VALUE_DOC(R10G10B10A2_UINT            , "")
    REFLECTION_ENUM_VALUE_DOC(R11G11B10_FLOAT             , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8B8A8_TYPELESS           , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8B8A8_UNORM              , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8B8A8_UNORM_SRGB         , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8B8A8_UINT               , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8B8A8_SNORM              , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8B8A8_SINT               , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16_TYPELESS             , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16_FLOAT                , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16_UNORM                , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16_UINT                 , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16_SNORM                , "")
    REFLECTION_ENUM_VALUE_DOC(R16G16_SINT                 , "")
    REFLECTION_ENUM_VALUE_DOC(R32_TYPELESS                , "")
    REFLECTION_ENUM_VALUE_DOC(D32_FLOAT                   , "")
    REFLECTION_ENUM_VALUE_DOC(R32_FLOAT                   , "")
    REFLECTION_ENUM_VALUE_DOC(R32_UINT                    , "")
    REFLECTION_ENUM_VALUE_DOC(R32_SINT                    , "")
    REFLECTION_ENUM_VALUE_DOC(R24G8_TYPELESS              , "")
    REFLECTION_ENUM_VALUE_DOC(D24_UNORM_S8_UINT           , "")
    REFLECTION_ENUM_VALUE_DOC(R24_UNORM_X8_TYPELESS       , "")
    REFLECTION_ENUM_VALUE_DOC(X24_TYPELESS_G8_UINT        , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8_TYPELESS               , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8_UNORM                  , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8_UINT                   , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8_SNORM                  , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8_SINT                   , "")
    REFLECTION_ENUM_VALUE_DOC(R16_TYPELESS                , "")
    REFLECTION_ENUM_VALUE_DOC(R16_FLOAT                   , "")
    REFLECTION_ENUM_VALUE_DOC(D16_UNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(R16_UNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(R16_UINT                    , "")
    REFLECTION_ENUM_VALUE_DOC(R16_SNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(R16_SINT                    , "")
    REFLECTION_ENUM_VALUE_DOC(R8_TYPELESS                 , "")
    REFLECTION_ENUM_VALUE_DOC(R8_UNORM                    , "")
    REFLECTION_ENUM_VALUE_DOC(R8_UINT                     , "")
    REFLECTION_ENUM_VALUE_DOC(R8_SNORM                    , "")
    REFLECTION_ENUM_VALUE_DOC(R8_SINT                     , "")
    REFLECTION_ENUM_VALUE_DOC(A8_UNORM                    , "")
    REFLECTION_ENUM_VALUE_DOC(R1_UNORM                    , "")
    REFLECTION_ENUM_VALUE_DOC(R9G9B9E5_SHAREDEXP          , "")
    REFLECTION_ENUM_VALUE_DOC(R8G8_B8G8_UNORM             , "")
    REFLECTION_ENUM_VALUE_DOC(G8R8_G8B8_UNORM             , "")
    REFLECTION_ENUM_VALUE_DOC(BC1_TYPELESS                , "")
    REFLECTION_ENUM_VALUE_DOC(BC1_UNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(BC1_UNORM_SRGB              , "")
    REFLECTION_ENUM_VALUE_DOC(BC2_TYPELESS                , "")
    REFLECTION_ENUM_VALUE_DOC(BC2_UNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(BC2_UNORM_SRGB              , "")
    REFLECTION_ENUM_VALUE_DOC(BC3_TYPELESS                , "")
    REFLECTION_ENUM_VALUE_DOC(BC3_UNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(BC3_UNORM_SRGB              , "")
    REFLECTION_ENUM_VALUE_DOC(BC4_TYPELESS                , "")
    REFLECTION_ENUM_VALUE_DOC(BC4_UNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(BC4_SNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(BC5_TYPELESS                , "")
    REFLECTION_ENUM_VALUE_DOC(BC5_UNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(BC5_SNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(B5G6R5_UNORM                , "")
    REFLECTION_ENUM_VALUE_DOC(B5G5R5A1_UNORM              , "")
    REFLECTION_ENUM_VALUE_DOC(B8G8R8A8_UNORM              , "")
    REFLECTION_ENUM_VALUE_DOC(B8G8R8X8_UNORM              , "")
    REFLECTION_ENUM_VALUE_DOC(R10G10B10_XR_BIAS_A2_UNORM  , "")
    REFLECTION_ENUM_VALUE_DOC(B8G8R8A8_TYPELESS           , "")
    REFLECTION_ENUM_VALUE_DOC(B8G8R8A8_UNORM_SRGB         , "")
    REFLECTION_ENUM_VALUE_DOC(B8G8R8X8_TYPELESS           , "")
    REFLECTION_ENUM_VALUE_DOC(B8G8R8X8_UNORM_SRGB         , "")
    REFLECTION_ENUM_VALUE_DOC(BC6H_TYPELESS               , "")
    REFLECTION_ENUM_VALUE_DOC(BC6H_UF16                   , "")
    REFLECTION_ENUM_VALUE_DOC(BC6H_SF16                   , "")
    REFLECTION_ENUM_VALUE_DOC(BC7_TYPELESS                , "")
    REFLECTION_ENUM_VALUE_DOC(BC7_UNORM                   , "")
    REFLECTION_ENUM_VALUE_DOC(BC7_UNORM_SRGB              , "")
    REFLECTION_ENUM_VALUE_DOC(AYUV                        , "")
    REFLECTION_ENUM_VALUE_DOC(Y410                        , "")
    REFLECTION_ENUM_VALUE_DOC(Y416                        , "")
    REFLECTION_ENUM_VALUE_DOC(NV12                        , "")
    REFLECTION_ENUM_VALUE_DOC(P010                        , "")
    REFLECTION_ENUM_VALUE_DOC(P016                        , "")
    //REFLECTION_ENUM_VALUE_DOC(420_OPAQUE                , "")
    REFLECTION_ENUM_VALUE_DOC(YUY2                        , "")
    REFLECTION_ENUM_VALUE_DOC(Y210                        , "")
    REFLECTION_ENUM_VALUE_DOC(Y216                        , "")
    REFLECTION_ENUM_VALUE_DOC(NV11                        , "")
    REFLECTION_ENUM_VALUE_DOC(AI44                        , "")
    REFLECTION_ENUM_VALUE_DOC(IA44                        , "")
    REFLECTION_ENUM_VALUE_DOC(P8                          , "")
    REFLECTION_ENUM_VALUE_DOC(A8P8                        , "")
    REFLECTION_ENUM_VALUE_DOC(B4G4R4A4_UNORM              , "")

REFLECTION_ENUM_END
//=============================================================================
}
}

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
rendering::BaseSurface const* InputSurfaceDescriptor::GetAsSurface(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_MSG(false,"Can not be used as surface");
    return nullptr;
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
rendering::BaseSurface const* OutputSurfaceDescriptor::GetAsSurface(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_MSG(false,"Can not be used as surface");
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
    if(rendering::SurfaceFormat::UNKNOWN == m_surfaceFormat)
    {
        properties.baseFormat = rendering::SurfaceFormat::R8G8B8A8_TYPELESS;
        properties.readFormat = rendering::SurfaceFormat::R8G8B8A8_UNORM_SRGB;
        properties.writeFormat = rendering::SurfaceFormat::R8G8B8A8_UNORM_SRGB;
    }
    else
    {
        rendering::SurfaceFormatProperties const& prop = GetProperties(m_surfaceFormat);
        properties.baseFormat = prop.typelessFormat;
        properties.readFormat = m_surfaceFormat;
        properties.writeFormat = m_surfaceFormat;
    }
    rendering::ResolutionServer const* resolutionServer = iCompositing->GetResolutionServer(m_resolution.get());
    SG_ASSERT(nullptr != resolutionServer);
    rendering::Surface* surface = new rendering::Surface(iCompositing->RenderDevice(), resolutionServer, &properties);
    return surface;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SurfaceDescriptor::VirtualOnCreated(reflection::ObjectCreationContext& iContext)
{
    reflection_parent_type::VirtualOnCreated(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void SurfaceDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    REFLECTION_CHECK_PROPERTY_NotNull(resolution);
    if(rendering::SurfaceFormat::UNKNOWN != m_surfaceFormat)
    {
        rendering::SurfaceFormatProperties const& prop = GetProperties(m_surfaceFormat);
        SG_ASSERT_AND_UNUSED(!prop.isBC);
        SG_ASSERT(prop.isUniformLayout);
        SG_ASSERT(prop.typelessFormat != m_surfaceFormat);
        SG_ASSERT(prop.channelCount > 0);
    }
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), SurfaceDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(resolution, "")
    REFLECTION_m_PROPERTY_DOC(mipLevels, "")
    REFLECTION_m_PROPERTY_DOC(surfaceFormat, "")
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
    // TODO: Make depth format configurable
#if 0
    {
        properties.baseFormat = rendering::SurfaceFormat::R16_TYPELESS;
        properties.readFormat = rendering::SurfaceFormat::R16_UNORM;
        properties.writeFormat = rendering::SurfaceFormat::D16_UNORM;
    }
#else
    {
        properties.baseFormat = rendering::SurfaceFormat::R32_TYPELESS;
        properties.readFormat = rendering::SurfaceFormat::R32_FLOAT;
        properties.writeFormat = rendering::SurfaceFormat::D32_FLOAT;
    }
#endif
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
rendering::BaseSurface const* TextureSurfaceDescriptor::GetAsSurface(Compositing* iCompositing) const
{
    SG_UNUSED(iCompositing);
    SG_ASSERT_MSG(false,"Can not be used as surface");
    return nullptr;
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

