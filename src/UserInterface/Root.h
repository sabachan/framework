#ifndef UserInterface_Root_H
#define UserInterface_Root_H

#include <Core/Observer.h>
#include <Math/Vector.h>
#include <System/UserInputListener.h>

namespace sg {
namespace rendering {
    class ShaderConstantDatabase;
}
namespace renderengine {
    class CompositingLayer;
}
namespace system {
    class Window;
}
}

namespace sg {
namespace ui {
//=============================================================================
class Component;
class LayerManager;
class RootContainer;
//=============================================================================
class Root : private system::IUserInputListener
           , private Observer<ObservableValue<uint2> >
{
public:
    Root(system::Window* iWindow, renderengine::CompositingLayer* iLayer);
    virtual ~Root();

    void Draw();
    virtual void OnUserInputEvent(system::UserInputEvent const& iEvent) override;
    virtual void VirtualOnNotified(ObservableValue<uint2> const* iObservable) override;

    void AddComponent(Component* iComponent, i32 iLayer = 0);
    void RemoveComponent(Component* iComponent);

private:
    scopedptr<RootContainer> m_container;
    scopedptr<LayerManager> m_layerManager;
    safeptr<system::Window> m_window;
    safeptr<renderengine::CompositingLayer> m_layer;
    SG_CODE_FOR_ASSERT(size_t m_pointerEventIndex;)
};
//=============================================================================
}
}

#endif
