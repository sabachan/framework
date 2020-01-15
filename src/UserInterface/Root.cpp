#include "stdafx.h"

#include "Root.h"

#include "Component.h"
#include "Container.h"
#include "Context.h"
#include "LayerManager.h"
#include "PointerEvent.h"
#include "RootContainer.h"
#include <Core/Log.h>
#include <Core/StringFormat.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <RenderEngine/CompositingLayer.h>
#include <System/KeyboardUtils.h>
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
    system::UserInputDeviceType const deviceType = iEvent.DeviceType();
    if(system::UserInputDeviceType::Mouse == deviceType)
    {
        SG_ASSERT(0 == iEvent.DeviceId());
#if 0 // SG_ENABLE_ASSERT
        system::MouseState mouseState(iEvent.State());
        SG_LOG_DEBUG("UI", Format("Pointer: (%0, %1)", mouseState.Position().x(), mouseState.Position().y()));
#endif
        PointerEventContext context;
        PointerEvent pointerEvent(iEvent SG_CODE_FOR_ASSERT(SG_COMMA m_pointerEventIndex));
        SG_CODE_FOR_ASSERT(++m_pointerEventIndex;)
        m_container->OnPointerEvent(context, pointerEvent);
    }
    else if(system::UserInputDeviceType::Keyboard == deviceType)
    {
        IFocusable* focusable = m_container->AsFocusableIFP();
        SG_ASSERT(nullptr != focusable);
        FocusableEvent focusableEvent(iEvent SG_CODE_FOR_ASSERT(SG_COMMA m_focusaleEventIndex));
        SG_CODE_FOR_ASSERT(++m_focusaleEventIndex;)
        focusable->OnFocusableEvent(focusableEvent);

        FocusDirection direction = FocusDirection::None;
        if(system::KeyboardShortcutAdapter::IsTriggerableEvent(iEvent))
        {
            std::pair<system::KeyboardShortcutAdapter, FocusDirection> const shortcuts[] = {
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Left,   false, false, false, system::KeyboardLayout::UserLayout), FocusDirection::Left),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Right,  false, false, false, system::KeyboardLayout::UserLayout), FocusDirection::Right),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Up,     false, false, false, system::KeyboardLayout::UserLayout), FocusDirection::Up),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Down,   false, false, false, system::KeyboardLayout::UserLayout), FocusDirection::Down),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Tab,    false, false, false, system::KeyboardLayout::UserLayout), FocusDirection::Next),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Tab,    true,  false, false, system::KeyboardLayout::UserLayout), FocusDirection::Previous),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Tab,    false, true,  false, system::KeyboardLayout::UserLayout), FocusDirection::MoveIn),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Tab,    true,  true,  false, system::KeyboardLayout::UserLayout), FocusDirection::MoveOut),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Space,  false, false, false, system::KeyboardLayout::UserLayout), FocusDirection::Activate),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Return, false, false, false, system::KeyboardLayout::UserLayout), FocusDirection::Validate),
                std::make_pair(system::KeyboardShortcutAdapter(system::KeyboardKey::Escape, false, false, false, system::KeyboardLayout::UserLayout), FocusDirection::Cancel),
            };
            for(auto const& s : AsArrayView(shortcuts))
            {
                if(s.first.IsTriggered_AssumeTriggerableEvent(iEvent))
                {
                    direction = s.second;
                    break;
                }
            }
            if(FocusDirection::None != direction)
            {
                bool const hasMoved = focusable->RequestMoveFocusReturnHasMoved(direction);
                if(hasMoved)
                    iEvent.SetMasked();
            }
        }
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
