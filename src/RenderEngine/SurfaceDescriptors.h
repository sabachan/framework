#ifndef RenderEngine_SurfaceDescriptors_H
#define RenderEngine_SurfaceDescriptors_H

#include <Core/FilePath.h>
#include <Core/SmartPtr.h>
#include <Reflection/BaseClass.h>
#include <Rendering/Surface.h>

namespace sg {
namespace rendering {
    class BaseSurface;
    class BCSurface;
    class DepthStencilSurface;
    class IRenderTarget;
    class IShaderResource;
    class Surface;
    class TextureFromFile;
    REFLECTION_ENUM_HEADER(SurfaceFormat)
}
}

namespace sg {
namespace renderengine {
//=============================================================================
class AbstractResolutionDescriptor;
class Compositing;
//=============================================================================
class AbstractSurfaceDescriptor : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(AbstractSurfaceDescriptor, reflection::BaseClass)
public:
    AbstractSurfaceDescriptor();
    virtual ~AbstractSurfaceDescriptor() override;
    virtual rendering::IRenderTarget const* GetAsRenderTarget(Compositing* iCompositing) const = 0;
    virtual rendering::IShaderResource const* GetAsShaderResource(Compositing* iCompositing) const = 0;
    virtual rendering::BaseSurface const* GetAsSurface(Compositing* iCompositing) const = 0;
protected:
    friend class Compositing;
    virtual rendering::BaseSurface* CreateSurface(Compositing* iCompositing) const = 0;
};
//=============================================================================
class InputSurfaceDescriptor : public AbstractSurfaceDescriptor
{
    REFLECTION_CLASS_HEADER(InputSurfaceDescriptor, AbstractSurfaceDescriptor)
public:
    InputSurfaceDescriptor();
    virtual ~InputSurfaceDescriptor() override;
    virtual rendering::IRenderTarget const* GetAsRenderTarget(Compositing* iCompositing) const override;
    virtual rendering::IShaderResource const* GetAsShaderResource(Compositing* iCompositing) const override;
    virtual rendering::BaseSurface const* GetAsSurface(Compositing* iCompositing) const override;
private:
    virtual rendering::BaseSurface* CreateSurface(Compositing* iCompositing) const override;
};
//=============================================================================
class OutputSurfaceDescriptor : public AbstractSurfaceDescriptor
{
    REFLECTION_CLASS_HEADER(OutputSurfaceDescriptor, AbstractSurfaceDescriptor)
public:
    OutputSurfaceDescriptor();
    virtual ~OutputSurfaceDescriptor() override;
    virtual rendering::IRenderTarget const* GetAsRenderTarget(Compositing* iCompositing) const override;
    virtual rendering::IShaderResource const* GetAsShaderResource(Compositing* iCompositing) const override;
    virtual rendering::BaseSurface const* GetAsSurface(Compositing* iCompositing) const override;
private:
    virtual rendering::BaseSurface* CreateSurface(Compositing* iCompositing) const override;
};
//=============================================================================
class SurfaceDescriptor : public AbstractSurfaceDescriptor
{
    REFLECTION_CLASS_HEADER(SurfaceDescriptor, AbstractSurfaceDescriptor)
public:
    enum class Format {

    };
public:
    SurfaceDescriptor();
    virtual ~SurfaceDescriptor() override;
    virtual rendering::IRenderTarget const* GetAsRenderTarget(Compositing* iCompositing) const override;
    virtual rendering::IShaderResource const* GetAsShaderResource(Compositing* iCompositing) const override;
    virtual rendering::Surface const* GetAsSurface(Compositing* iCompositing) const override;
    AbstractResolutionDescriptor const* ResolutionDescriptor() const { return m_resolution.get(); }
private:
    virtual rendering::Surface* CreateSurface(Compositing* iCompositing) const override;
    virtual void VirtualOnCreated(reflection::ObjectCreationContext& iContext) override;
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    refptr<AbstractResolutionDescriptor> m_resolution;
    size_t m_mipLevels;
    rendering::SurfaceFormat m_surfaceFormat { rendering::SurfaceFormat::UNKNOWN };
};
//=============================================================================
class BCSurfaceDescriptor : public AbstractSurfaceDescriptor
{
    REFLECTION_CLASS_HEADER(BCSurfaceDescriptor, AbstractSurfaceDescriptor)
public:
    BCSurfaceDescriptor();
    virtual ~BCSurfaceDescriptor() override;
    virtual rendering::IRenderTarget const* GetAsRenderTarget(Compositing* iCompositing) const override;
    virtual rendering::IShaderResource const* GetAsShaderResource(Compositing* iCompositing) const override;
    virtual rendering::BCSurface const* GetAsSurface(Compositing* iCompositing) const override;
private:
    virtual rendering::BCSurface* CreateSurface(Compositing* iCompositing) const override;
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    refptr<AbstractResolutionDescriptor> m_resolution;
    size_t m_mipLevels;
};
//=============================================================================
class DepthStencilSurfaceDescriptor : public AbstractSurfaceDescriptor
{
    REFLECTION_CLASS_HEADER(DepthStencilSurfaceDescriptor, AbstractSurfaceDescriptor)
public:
    DepthStencilSurfaceDescriptor();
    virtual ~DepthStencilSurfaceDescriptor() override;
    virtual rendering::IRenderTarget const* GetAsRenderTarget(Compositing* iCompositing) const override;
    virtual rendering::IShaderResource const* GetAsShaderResource(Compositing* iCompositing) const override;
    virtual rendering::DepthStencilSurface const* GetAsSurface(Compositing* iCompositing) const override;
private:
    virtual rendering::DepthStencilSurface* CreateSurface(Compositing* iCompositing) const override;
#if ENABLE_REFLECTION_PROPERTY_CHECK
    virtual void VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const override;
#endif
private:
    refptr<AbstractResolutionDescriptor> m_resolution;
    size_t m_mipLevels;
};
//=============================================================================
class TextureSurfaceDescriptor : public AbstractSurfaceDescriptor
{
    REFLECTION_CLASS_HEADER(TextureSurfaceDescriptor, AbstractSurfaceDescriptor)
public:
    TextureSurfaceDescriptor();
    virtual ~TextureSurfaceDescriptor() override;
    virtual rendering::IRenderTarget const* GetAsRenderTarget(Compositing* iCompositing) const override;
    virtual rendering::IShaderResource const* GetAsShaderResource(Compositing* iCompositing) const override;
    virtual rendering::BaseSurface const* GetAsSurface(Compositing* iCompositing) const override;
    rendering::TextureFromFile const* CreateTexture(Compositing* iCompositing) const;
private:
    virtual rendering::BaseSurface* CreateSurface(Compositing* iCompositing) const override;
private:
    FilePath m_filename;
};
//=============================================================================
}
}

#endif
