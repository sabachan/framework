#ifndef AppUtils_ApplicationLauncher_H
#define AppUtils_ApplicationLauncher_H

#include <Core/Observer.h>
#include <Core/Singleton.h>
#include <Core/TimeFromWallClock.h>
#include <Core/ArrayView.h>
#include <Reflection/BaseClass.h>
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
namespace toolsui {
    class Button;
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
    typedef void (*LaunchFct)(void*);
    LaunchFct launch;
    void* arg;
};
//=============================================================================
class ILaunchable : public reflection::BaseClass
{
    REFLECTION_CLASS_HEADER(ILaunchable, reflection::BaseClass)
public:
    virtual void Launch() = 0;
    char const* Name() const { return m_name.c_str(); }
private:
    std::string m_name;
};
//=============================================================================
void Append(ArrayList<ApplicationDescriptor>& oDescriptors, ArrayView<ApplicationDescriptor const> iDescriptors);
void Append(ArrayList<ApplicationDescriptor>& oDescriptors, ILaunchable* iLaunchable);
void AppendLaunchablesInObjectScript(ArrayList<ApplicationDescriptor>& oDescriptors, ArrayList<refptr<ILaunchable>>& oLifeHandler, FilePath const& iFilepath);
//=============================================================================
class ApplicationLauncher : public system::WindowedApplication
                          , private system::IUserInputListener
                          , private Observer<ObservableValue<uint2> >
{
public:
    ApplicationLauncher();
    virtual ~ApplicationLauncher() override;

    size_t RunReturnNextAppIndex(sg::ArrayView<sg::ApplicationDescriptor const> const& iAppDescriptors);
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
#if SG_ENABLE_TOOLS
class ApplicationLauncherV2 : public system::WindowedApplication
                            , private system::IUserInputListener
                            , private Observer<ObservableValue<uint2> >
                            , private Observer<toolsui::Button>
{
public:
    ApplicationLauncherV2();
    virtual ~ApplicationLauncherV2() override;

    size_t RunReturnNextAppIndex(sg::ArrayView<sg::ApplicationDescriptor const> const& iAppDescriptors);
private:
    virtual void VirtualOneTurn() override;
    virtual void OnUserInputEvent(system::UserInputEvent const& iEvent) override;
    virtual void VirtualOnNotified(ObservableValue<uint2> const* iObservable) override;
    virtual void VirtualOnNotified(toolsui::Button const* iObservable) override;
    virtual void VirtualOnDropFile(char const* iFilePath, system::Window* iWindow, uint2 const& iPosition) override;
public:
    void OpenFile(FilePath const& iPath);
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

    size_t m_nextAppIndex;
    refptr<ILaunchable> m_launchable;
};
#endif
//=============================================================================
}

#endif
