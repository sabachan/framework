#ifndef UserInterface_OffscreenContainer_H
#define UserInterface_OffscreenContainer_H

#include <Core/Singleton.h>
#include <Rendering/ResolutionServer.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <Rendering/Surface.h>
#include <Reflection/BaseClass.h>
#include "Container.h"

namespace sg {
namespace rendering {
    class IShaderResource;
    class RenderDevice;
    class Surface;
}
namespace renderengine {
    class CompositingDescriptor;
    class CompositingLayer;
    class ICompositing;
}
namespace ui {
    class LayerManager;
}
}

namespace sg {
namespace ui {
//=============================================================================
class OffscreenCommon : public reflection::BaseClass
                      , public Singleton<OffscreenCommon>
{
    PARENT_SAFE_COUNTABLE(reflection::BaseClass)
    SG_NON_COPYABLE(OffscreenCommon)
    REFLECTION_CLASS_HEADER(OffscreenCommon, reflection::BaseClass)
public:
    OffscreenCommon();
    renderengine::CompositingDescriptor const* GetCompositingDescriptor() { return m_compositingDescriptor.get(); }
public:
    rendering::ShaderConstantName const name_viewport_resolution;
private:
    refptr<renderengine::CompositingDescriptor const> m_compositingDescriptor;
};
//=============================================================================
class OffscreenContainer : public Container
{
    typedef Container parent_type;
public:
    OffscreenContainer(rendering::SurfaceProperties const* iOptionnalProperties = nullptr);
    virtual ~OffscreenContainer() override;
    rendering::IShaderResource const* GetOffscreenTexture() { return m_surface.get(); }
protected:
    virtual void VirtualOnDraw(DrawContext const& iContext) override;
    virtual void VirtualOnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iPointerEvent) override;
    virtual void VirtualInvalidateChildrenPlacement() override;
private:
    void UpdateResolutionIFN();
private:
    rendering::ResolutionServer m_resolution;
    refptr<rendering::ShaderConstantDatabase> m_shaderConstantDatabase;
    refptr<rendering::Surface> m_surface;
    refptr<renderengine::ICompositing> m_compositing;
    safeptr<renderengine::CompositingLayer> m_layer;
    scopedptr<ui::LayerManager> m_layerManager;
    bool m_mayNeedUpdateResolution;
};
//=============================================================================
}
}

#endif
