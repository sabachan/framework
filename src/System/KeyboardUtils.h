#ifndef System_KeyboardUtils_H
#define System_KeyboardUtils_H

#include <Core/Platform.h>
#include <Core/IntTypes.h>
#include <Core/Tribool.h>

namespace sg {
namespace system {
//=============================================================================
class UserInputEvent;
//=============================================================================
enum class KeyboardKey
{
    Backspace,
    Tab,
    Return,
    Shift,
    Control,
    LeftShift,
    LeftControl,
    RightShift,
    RightControl,
    Menu,
    Pause,
    CapsLock,
    Escape,
    Space,
    PageUp,
    PageNext,
    End,
    Home,
    Left,
    Up,
    Right,
    Down,
    PrintScreen,
    Insert,
    Delete,
    Key_0,
    Key_1,
    Key_2,
    Key_3,
    Key_4,
    Key_5,
    Key_6,
    Key_7,
    Key_8,
    Key_9,
    Key_A,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,
    LeftWin,
    RightWin,
    Apps,
    Numpad_0,
    Numpad_1,
    Numpad_2,
    Numpad_3,
    Numpad_4,
    Numpad_5,
    Numpad_6,
    Numpad_7,
    Numpad_8,
    Numpad_9,
    Multiply,
    Add,
    Subtract,
    Decimal,
    Divide,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    Count,
};
//=============================================================================
enum class KeyboardModifier
{
    Shift = 0,
    Control = 1,
    Alt = 2,
};
//=============================================================================
enum class KeyboardLayout { QWERTY, UserLayout };
//=============================================================================
struct KeyboardState
{
public:
    static u8 const MOUSE_BUTTONS_MASK = 0x1F;
public:
    KeyboardState() : data(0) {}
    explicit KeyboardState(u64 iData) : data(iData) {}
    bool IsModifierOn(KeyboardModifier m) const { return 0 != (data & (u64(1) << size_t(m))); }
    void SetModifier(KeyboardModifier m, bool b) { if(b) { data |= u64(1) << size_t(m); } else { data &= ~(u64(1) << size_t(m)); } }
    u64 Data() const { return data; }
private:
    u64 data;
};
//=============================================================================
class KeyboardEntryAdapter
{
public:
    KeyboardEntryAdapter();
    KeyboardEntryAdapter(KeyboardKey iKey, KeyboardLayout iLayout);

    void SetKey(KeyboardKey iKey, KeyboardLayout iLayout);
    bool IsValid() const { return -1 != m_code; }
    bool DoesMatch(UserInputEvent const& iEvent) const;
    static bool IsKeyEvent(UserInputEvent const& iEvent);
    bool DoesMatch_AssumeKeyEvent(UserInputEvent const& iEvent) const;
private:
#if SG_PLATFORM_IS_WIN
    bool m_isScanCodeElseVirtualKey;
    u32 m_code;
#endif
};
//=============================================================================
class KeyboardEntryStateAdapter
{
    SG_NON_COPYABLE(KeyboardEntryStateAdapter)
public:
    KeyboardEntryStateAdapter() {}
    KeyboardEntryStateAdapter(KeyboardKey iKey, KeyboardLayout iLayout) : m_entryAdapter(iKey, iLayout) {}

    void SetKey(KeyboardKey iKey, KeyboardLayout iLayout) { m_entryAdapter.SetKey(iKey, iLayout); m_state = false; }
    bool IsValid() const { return m_entryAdapter.IsValid(); }
    void Update(UserInputEvent const& iEvent);
    bool State() const { return m_state; };
private:
    KeyboardEntryAdapter m_entryAdapter;
    bool m_state { false };
};
//=============================================================================
class KeyboardShortcutAdapter
{
public:
    KeyboardShortcutAdapter();
    KeyboardShortcutAdapter(KeyboardKey iKey, tribool shift, tribool ctrl, tribool alt, KeyboardLayout iMode);

    bool IsTriggered(UserInputEvent const& iEvent) const;
    static bool IsTriggerableEvent(UserInputEvent const& iEvent);
    bool IsTriggered_AssumeTriggerableEvent(UserInputEvent const& iEvent) const;

    bool IsValid() const { return m_entry.IsValid(); }
    bool DoesMatch(UserInputEvent const& iEvent) const;
    static bool IsKeyEvent(UserInputEvent const& iEvent) { return KeyboardEntryAdapter::IsKeyEvent(iEvent); }
    bool DoesMatch_AssumeKeyEvent(UserInputEvent const& iEvent) const;
private:
    KeyboardEntryAdapter m_entry;
    tribool m_shift;
    tribool m_ctrl;
    tribool m_alt;
};
//=============================================================================
}
}

#endif
