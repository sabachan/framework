#ifndef ToolsUI_DemoApplication_H
#define ToolsUI_DemoApplication_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include <Core/Observer.h>
#include <Core/Singleton.h>
#include <Core/ArrayView.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <System/WindowedApplication.h>

namespace sg {
//=============================================================================
namespace rendering {
    class RenderDevice;
    class RenderWindow;
    class ShaderConstantDatabase;
}
namespace renderengine {
    class ICompositing;
    class CompositingLayer;
}
namespace system {
    class UserInputEvent;
}
namespace ui {
    class TypefaceFromBitMapFont;
    class Root;
    class TextureDrawer;
    class UniformDrawer;
}
//=============================================================================
}

namespace sg {
namespace toolsui {
//=============================================================================
class DemoApplication : public system::WindowedApplication
                      , private system::IUserInputListener
                      , private Observer<ObservableValue<uint2> >
{
public:
    DemoApplication();
    virtual ~DemoApplication() override;

    void Run();
private:
    virtual void VirtualOneTurn() override;
    virtual void OnUserInputEvent(system::UserInputEvent const& iEvent) override;
    virtual void VirtualOnNotified(ObservableValue<uint2> const* iObservable) override;

private:
    std::vector<refptr<system::Window> > m_windowHandles;
    scopedptr<rendering::RenderDevice> m_renderDevice;
    std::vector<refptr<rendering::RenderWindow> > m_renderWindows;

    refptr<rendering::ShaderConstantDatabase> m_shaderConstantDatabase;
    refptr<renderengine::ICompositing> m_compositing;
    safeptr<renderengine::CompositingLayer> m_layer;
    scopedptr<ui::Root> m_uiRoot;

    rendering::ShaderConstantName name_viewport_resolution;
};
//=============================================================================
}
}

#endif
