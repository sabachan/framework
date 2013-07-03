#ifndef AppUtils_ApplicationLauncher_H
#define AppUtils_ApplicationLauncher_H

#include <Core/Observer.h>
#include <Core/Singleton.h>
#include <Core/TimeFromWallClock.h>
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
struct ApplicationDescriptor
{
    char const* name;
    typedef void (*LaunchFct)();
    LaunchFct launch;
};
//=============================================================================
class ApplicationLauncher : public system::WindowedApplication
                          , private system::IUserInputListener
                          , private Observer<ObservableValue<uint2> >
{
public:
    ApplicationLauncher();
    virtual ~ApplicationLauncher() override;

    size_t RunReturnNextAppIndex(sg::ArrayView<sg::ApplicationDescriptor> const& iAppDescriptors);
private:
    virtual void VirtualOneTurn() override;
    virtual void OnUserInputEvent(system::UserInputEvent const& iEvent) override;
    virtual void VirtualOnNotified(ObservableValue<uint2> const* iObservable) override;
public:
    class UITimeServer : public TimeFromWallClock
                       , public Singleton<UITimeServer>
    {
        PARENT_SAFE_COUNTABLE(TimeFromWallClock);
    };
private:
    std::vector<refptr<system::Window> > m_windowHandles;
    scopedptr<rendering::RenderDevice> m_renderDevice;
    std::vector<refptr<rendering::RenderWindow> > m_renderWindows;

    refptr<rendering::ShaderConstantDatabase> m_shaderConstantDatabase;
    refptr<renderengine::ICompositing> m_compositing;
    safeptr<renderengine::CompositingLayer> m_layer;
    scopedptr<ui::Root> m_uiRoot;

    rendering::ShaderConstantName name_viewport_resolution;
    UITimeServer m_uiTimeServer;
};
//=============================================================================
}

#endif
