#include "stdafx.h"

#include "KeyboardUtils.h"

#include "UserInputEvent.h"

namespace sg {
namespace system {
//=============================================================================
#if SG_PLATFORM_IS_WIN
namespace {
    struct KeyboardKeyTraits
    {
        SG_CODE_FOR_ASSERT(KeyboardKey key;)
        u32 scanCodeForQWERTY;
        u32 virtualKey;
        bool extended;
    };
    KeyboardKeyTraits const keyboardKeyTraits[] =
    {
        { SG_CODE_FOR_ASSERT(KeyboardKey::Backspace    SG_COMMA)      8,            u32(VK_BACK),     false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Tab          SG_COMMA)      9,            u32(VK_TAB),      false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Return       SG_COMMA)      13,           u32(VK_RETURN),   false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Shift        SG_COMMA)      all_ones,     u32(VK_SHIFT),    false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Control      SG_COMMA)      all_ones,     u32(VK_CONTROL),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::LeftShift    SG_COMMA)      42,           u32(all_ones),    false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::LeftControl  SG_COMMA)      29,           u32(all_ones),    false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::RightShift   SG_COMMA)      54,           u32(all_ones),    false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::RightControl SG_COMMA)      29,           u32(all_ones),    true  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Menu         SG_COMMA)      all_ones,     u32(VK_MENU),     false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Pause        SG_COMMA)      all_ones,     u32(VK_PAUSE),    false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::CapsLock     SG_COMMA)      all_ones,     u32(VK_CAPITAL),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Escape       SG_COMMA)      all_ones,     u32(VK_ESCAPE),   false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Space        SG_COMMA)      all_ones,     u32(VK_SPACE),    false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::PageUp       SG_COMMA)      all_ones,     u32(VK_PRIOR),    false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::PageNext     SG_COMMA)      all_ones,     u32(VK_NEXT),     false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::End          SG_COMMA)      all_ones,     u32(VK_END),      false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Home         SG_COMMA)      all_ones,     u32(VK_HOME),     false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Left         SG_COMMA)      all_ones,     u32(VK_LEFT),     false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Up           SG_COMMA)      all_ones,     u32(VK_UP),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Right        SG_COMMA)      all_ones,     u32(VK_RIGHT),    false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Down         SG_COMMA)      all_ones,     u32(VK_DOWN),     false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::PrintScreen  SG_COMMA)      all_ones,     u32(VK_SNAPSHOT), false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Insert       SG_COMMA)      all_ones,     u32(VK_INSERT),   false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Delete       SG_COMMA)      all_ones,     u32(VK_DELETE),   false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_0        SG_COMMA)      all_ones,     u32('0'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_1        SG_COMMA)      all_ones,     u32('1'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_2        SG_COMMA)      all_ones,     u32('2'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_3        SG_COMMA)      all_ones,     u32('3'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_4        SG_COMMA)      all_ones,     u32('4'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_5        SG_COMMA)      all_ones,     u32('5'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_6        SG_COMMA)      all_ones,     u32('6'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_7        SG_COMMA)      all_ones,     u32('7'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_8        SG_COMMA)      all_ones,     u32('8'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_9        SG_COMMA)      all_ones,     u32('9'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_A        SG_COMMA)            30,     u32('A'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_B        SG_COMMA)            48,     u32('B'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_C        SG_COMMA)            46,     u32('C'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_D        SG_COMMA)            32,     u32('D'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_E        SG_COMMA)            18,     u32('E'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_F        SG_COMMA)            33,     u32('F'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_G        SG_COMMA)            34,     u32('G'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_H        SG_COMMA)            35,     u32('H'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_I        SG_COMMA)            23,     u32('I'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_J        SG_COMMA)            36,     u32('J'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_K        SG_COMMA)            37,     u32('K'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_L        SG_COMMA)            38,     u32('L'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_M        SG_COMMA)            50,     u32('M'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_N        SG_COMMA)            49,     u32('N'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_O        SG_COMMA)            24,     u32('O'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_P        SG_COMMA)            25,     u32('P'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_Q        SG_COMMA)            16,     u32('Q'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_R        SG_COMMA)            19,     u32('R'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_S        SG_COMMA)            31,     u32('S'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_T        SG_COMMA)            20,     u32('T'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_U        SG_COMMA)            22,     u32('U'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_V        SG_COMMA)            47,     u32('V'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_W        SG_COMMA)            17,     u32('W'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_X        SG_COMMA)            45,     u32('X'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_Y        SG_COMMA)            21,     u32('Y'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_Z        SG_COMMA)            44,     u32('Z'),         false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::LeftWin      SG_COMMA)      all_ones,     u32(VK_LWIN),     false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::RightWin     SG_COMMA)      all_ones,     u32(VK_RWIN),     false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Apps         SG_COMMA)      all_ones,     u32(VK_APPS),     false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_0     SG_COMMA)      all_ones,     u32(VK_NUMPAD0),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_1     SG_COMMA)      all_ones,     u32(VK_NUMPAD1),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_2     SG_COMMA)      all_ones,     u32(VK_NUMPAD2),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_3     SG_COMMA)      all_ones,     u32(VK_NUMPAD3),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_4     SG_COMMA)      all_ones,     u32(VK_NUMPAD4),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_5     SG_COMMA)      all_ones,     u32(VK_NUMPAD5),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_6     SG_COMMA)      all_ones,     u32(VK_NUMPAD6),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_7     SG_COMMA)      all_ones,     u32(VK_NUMPAD7),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_8     SG_COMMA)      all_ones,     u32(VK_NUMPAD8),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_9     SG_COMMA)      all_ones,     u32(VK_NUMPAD9),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Multiply     SG_COMMA)      all_ones,     u32(VK_MULTIPLY), false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Add          SG_COMMA)      all_ones,     u32(VK_ADD),      false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Subtract     SG_COMMA)      all_ones,     u32(VK_SUBTRACT), false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Decimal      SG_COMMA)      all_ones,     u32(VK_DECIMAL),  false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Divide       SG_COMMA)      all_ones,     u32(VK_DIVIDE),   false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F1           SG_COMMA)      all_ones,     u32(VK_F1),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F2           SG_COMMA)      all_ones,     u32(VK_F2),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F3           SG_COMMA)      all_ones,     u32(VK_F3),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F4           SG_COMMA)      all_ones,     u32(VK_F4),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F5           SG_COMMA)      all_ones,     u32(VK_F5),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F6           SG_COMMA)      all_ones,     u32(VK_F6),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F7           SG_COMMA)      all_ones,     u32(VK_F7),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F8           SG_COMMA)      all_ones,     u32(VK_F8),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F9           SG_COMMA)      all_ones,     u32(VK_F9),       false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F10          SG_COMMA)      all_ones,     u32(VK_F10),      false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F11          SG_COMMA)      all_ones,     u32(VK_F11),      false },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F12          SG_COMMA)      all_ones,     u32(VK_F12),      false },

        { SG_CODE_FOR_ASSERT(KeyboardKey::Count        SG_COMMA)      all_ones,     all_ones,         false },
    };
}
#endif
//=============================================================================
KeyboardEntryAdapter::KeyboardEntryAdapter()
#if SG_PLATFORM_IS_WIN
: m_code(all_ones)
, m_isScanCodeElseVirtualKey(false)
#else
#error "todo"
#endif
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
KeyboardEntryAdapter::KeyboardEntryAdapter(KeyboardKey iKey, KeyboardLayout iLayout)
{
    SetKey(iKey, iLayout);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void KeyboardEntryAdapter::SetKey(KeyboardKey iKey, KeyboardLayout iLayout)
{
#if SG_PLATFORM_IS_WIN
    SG_ASSERT(static_cast<size_t>(iKey) < SG_ARRAYSIZE(keyboardKeyTraits));
    KeyboardKeyTraits const& traits = keyboardKeyTraits[static_cast<size_t>(iKey)];
    SG_ASSERT(traits.key == iKey);
    switch(iLayout)
    {
    case KeyboardLayout::QWERTY:
        {
            SG_ASSERT_MSG(all_ones != traits.scanCodeForQWERTY, "This key is not supported in this layout");
            m_code = traits.scanCodeForQWERTY;
            m_isScanCodeElseVirtualKey = true;
        }
        break;
    case KeyboardLayout::UserLayout:
        {
            SG_ASSERT_MSG(all_ones != traits.virtualKey, "This key is not supported in this layout");
            m_code = traits.virtualKey;
            m_isScanCodeElseVirtualKey = false;
        }
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
#else
#error "todo"
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool KeyboardEntryAdapter::IsKeyEvent(UserInputEvent const& iEvent)
{
    if(UserInputDeviceType::Keyboard != iEvent.DeviceType())
        return false;
    if(UserInputEventType::OffToOn != iEvent.EventType()
       && UserInputEventType::OnRepeat != iEvent.EventType()
       && UserInputEventType::OnToOff != iEvent.EventType())
       return false;
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool KeyboardEntryAdapter::DoesMatch(UserInputEvent const& iEvent) const
{
    if(!IsKeyEvent(iEvent))
        return false;
    return DoesMatch_AssumeKeyEvent(iEvent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool KeyboardEntryAdapter::DoesMatch_AssumeKeyEvent(UserInputEvent const& iEvent) const
{
    SG_ASSERT(UserInputDeviceType::Keyboard == iEvent.DeviceType());
    SG_ASSERT(UserInputEventType::OffToOn == iEvent.EventType()
       || UserInputEventType::OnRepeat == iEvent.EventType()
       || UserInputEventType::OnToOff == iEvent.EventType());
    SG_ASSERT_MSG(0 == iEvent.DeviceId(), "not supported");
    u32 inputCode = m_isScanCodeElseVirtualKey ? iEvent.EntryId() : iEvent.AdditionalData();
    SG_ASSERT(-1 != inputCode);
    return m_code == inputCode;
}
//=============================================================================
void KeyboardEntryStateAdapter::Update(UserInputEvent const& iEvent)
{
    if(system::UserInputEventType::ResetAll == iEvent.EventType())
    {
        m_state = false;
    }
    else if(UserInputDeviceType::Keyboard == iEvent.DeviceType())
    {
        if(system::UserInputEventType::ResetDevice == iEvent.EventType())
        {
            m_state = false;
        }
        else
        {
            if(UserInputEventType::OffToOn == iEvent.EventType())
            {
                if(m_entryAdapter.DoesMatch_AssumeKeyEvent(iEvent))
                {
                    SG_ASSERT(!m_state);
                    if(!iEvent.IsMasked())
                    {
                        m_state = true;
                        iEvent.SetMasked();
                    }
                }
            }
            else if(UserInputEventType::OnRepeat == iEvent.EventType())
            {
                if(m_state && m_entryAdapter.DoesMatch_AssumeKeyEvent(iEvent))
                {
                    if(iEvent.IsMasked())
                        m_state = false;
                    else
                        iEvent.SetMasked();
                }
            }
            else if(UserInputEventType::OnToOff == iEvent.EventType())
            {
                if(m_state && m_entryAdapter.DoesMatch_AssumeKeyEvent(iEvent))
                {
                    m_state = false;
                    if(!iEvent.IsMasked())
                        iEvent.SetMasked();
                }
            }
        }
    }
}
//=============================================================================
KeyboardShortcutAdapter::KeyboardShortcutAdapter(KeyboardKey iKey, tribool shift, tribool ctrl, tribool alt, KeyboardLayout iMode)
    : m_entry(iKey, iMode)
    , m_shift(shift)
    , m_ctrl(ctrl)
    , m_alt(alt)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool KeyboardShortcutAdapter::IsTriggered(UserInputEvent const& iEvent) const
{
    if(iEvent.IsMasked())
        return false;
    if(!IsKeyEvent(iEvent))
        return false;
    if(iEvent.EventType() != UserInputEventType::OffToOn && iEvent.EventType() != UserInputEventType::OnRepeat)
        return false;
    return DoesMatch_AssumeKeyEvent(iEvent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool KeyboardShortcutAdapter::IsTriggerableEvent(UserInputEvent const& iEvent)
{
    if(iEvent.IsMasked())
        return false;
    if(!IsKeyEvent(iEvent))
        return false;
    if(iEvent.EventType() != UserInputEventType::OffToOn && iEvent.EventType() != UserInputEventType::OnRepeat)
        return false;
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool KeyboardShortcutAdapter::IsTriggered_AssumeTriggerableEvent(UserInputEvent const& iEvent) const
{
    SG_ASSERT(!iEvent.IsMasked());
    SG_ASSERT(IsKeyEvent(iEvent));
    SG_ASSERT(iEvent.EventType() == UserInputEventType::OffToOn || iEvent.EventType() == UserInputEventType::OnRepeat);
    return DoesMatch_AssumeKeyEvent(iEvent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool KeyboardShortcutAdapter::DoesMatch(UserInputEvent const& iEvent) const
{
    if(!IsKeyEvent(iEvent))
        return false;
    return DoesMatch_AssumeKeyEvent(iEvent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool KeyboardShortcutAdapter::DoesMatch_AssumeKeyEvent(UserInputEvent const& iEvent) const
{
    SG_ASSERT(UserInputDeviceType::Keyboard == iEvent.DeviceType());
    SG_ASSERT(UserInputEventType::OffToOn == iEvent.EventType()
       || UserInputEventType::OnRepeat == iEvent.EventType()
       || UserInputEventType::OnToOff == iEvent.EventType());
    SG_ASSERT_MSG(0 == iEvent.DeviceId(), "not supported");
    bool const doesEntryMatch = m_entry.DoesMatch_AssumeKeyEvent(iEvent);
    KeyboardState const state = KeyboardState(iEvent.State());
    auto DoesMatch = [](bool b, tribool t) { return t.Is(b) || t.Is(indeterminate); };
    bool const doModifiersMatch = DoesMatch(state.IsModifierOn(KeyboardModifier::Shift), m_shift)
        && DoesMatch(state.IsModifierOn(KeyboardModifier::Control), m_ctrl)
        && DoesMatch(state.IsModifierOn(KeyboardModifier::Alt), m_alt);
    return doesEntryMatch && doModifiersMatch;
}
//=============================================================================
}
}
