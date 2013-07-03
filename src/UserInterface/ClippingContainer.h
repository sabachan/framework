#ifndef UserInterface_ClippingContainer_H
#define UserInterface_ClippingContainer_H

#include <Core/Singleton.h>
#include <Rendering/ResolutionServer.h>
#include "OffscreenContainer.h"
#include "LayerManager.h"

namespace sg {
namespace rendering {
    class RenderDevice;
    class Surface;
}
namespace renderengine {
    class CompositingDescriptor;
    class CompositingLayer;
    class ICompositing;
}
namespace ui {
    class LayerManger;
    class TextureDrawer;
}
}

namespace sg {
namespace ui {
//=============================================================================
class ClippingContainer : public OffscreenContainer
{
    typedef OffscreenContainer parent_type;
public:
    ClippingContainer(ui::TextureDrawer const* iTextureDrawer);
    virtual ~ClippingContainer() override;
protected:
    virtual void VirtualOnDraw(DrawContext const& iContext) override;
private:
    safeptr<ui::TextureDrawer const> m_textureDrawer;
};
//=============================================================================
}
}

#endif
