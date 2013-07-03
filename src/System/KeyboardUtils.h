#ifndef System_KeyboardUtils_H
#define System_KeyboardUtils_H

#include <Core/Platform.h>
#include <Core/IntTypes.h>

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
};
//=============================================================================
enum class KeyboardLayout { QWERTY, UserLayout };
//=============================================================================
class KeyboardEntryAdapter
{
public:
    KeyboardEntryAdapter();
    KeyboardEntryAdapter(KeyboardKey iKey, KeyboardLayout iMode);

    bool IsValid() const { return -1 != m_code; }
    bool DoesMatch(UserInputEvent const& iEvent) const;
    bool DoesMatch_AssumeKeyEvent(UserInputEvent const& iEvent) const;
private:
#if SG_PLATFORM_IS_WIN
    bool m_isScanCodeElseVirtualKey;
    u32 m_code;
#endif
};
//=============================================================================
}
}

#endif
