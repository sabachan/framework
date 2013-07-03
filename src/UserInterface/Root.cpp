#include "stdafx.h"

#include "Root.h"

#include "Component.h"
#include "Container.h"
#include "Context.h"
#include "LayerManager.h"
#include "PointerEvent.h"
#include <Core/Log.h>
#include <Core/StringFormat.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <RenderEngine/CompositingLayer.h>
#include <System/Window.h>

namespace sg {
namespace ui {
//=============================================================================
Root::Root(system::Window* iWindow, renderengine::CompositingLayer* iLayer)
: m_container(new RootContainer)
, m_layerManager(new LayerManager)
, m_layer(iLayer)
, m_window(iWindow)
SG_CODE_FOR_ASSERT(SG_COMMA m_pointerEventIndex(0))
{
    SG_ASSERT(nullptr != iLayer);
    SG_ASSERT(nullptr != iWindow);
    m_window->ClientSize().RegisterObserver(this);
    VirtualOnNotified(&m_window->ClientSize());
    size_t const userInputLayer = 100; // TODO: How to fix this value ?
    m_window->RegisterUserInputListener(this, userInputLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Root::~Root()
{
    m_window->UnregisterUserInputListener(this);
    m_window->ClientSize().UnregisterObserver(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Root::Draw()
{
    uint2 const windowSize = m_window->ClientSize().Get();
    box2f const clippingBox = box2f::FromMinMax(float2(0), float2(windowSize));
    DrawContext context(*m_layerManager, m_layer.get(), clippingBox);
    m_container->OnDraw(context);
    m_layerManager->Clear();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Root::OnUserInputEvent(system::UserInputEvent const& iEvent)
{
    // TODO:
    // - PointerEvent
    // - FocusableEvent
    if(system::UserInputDeviceType::Mouse == iEvent.DeviceType())
    {
        SG_ASSERT(0 == iEvent.DeviceId());
#if 0 // SG_ENABLE_ASSERT
        system::MouseState mouseState(iEvent.State());
        SG_LOG_DEBUG(Format("Pointer: (%0, %1)", mouseState.Position().x(), mouseState.Position().y()));
#endif
        PointerEventContext context;
        PointerEvent pointerEvent(iEvent SG_CODE_FOR_ASSERT(SG_COMMA m_pointerEventIndex));
        SG_CODE_FOR_ASSERT(++m_pointerEventIndex;)
        m_container->OnPointerEvent(context, pointerEvent);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Root::VirtualOnNotified(ObservableValue<uint2> const* iObservable)
{
    float2 wh = float2(iObservable->Get());
    m_container->SetPlacementBox(box2f::FromMinDelta(float2(0,0), wh));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Root::AddComponent(Component* iComponent, i32 iLayer)
{
    m_container->RequestAddToFront(iComponent, iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Root::RemoveComponent(Component* iComponent)
{
    m_container->RequestRemove(iComponent);
}
//=============================================================================
}
}
