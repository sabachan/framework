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
    };
    KeyboardKeyTraits const keyboardKeyTraits[] =
    {
        { SG_CODE_FOR_ASSERT(KeyboardKey::Backspace    SG_COMMA)      all_ones,     u32(VK_BACK)     },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Tab          SG_COMMA)      all_ones,     u32(VK_TAB)      },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Return       SG_COMMA)      all_ones,     u32(VK_RETURN)   },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Shift        SG_COMMA)      all_ones,     u32(VK_SHIFT)    },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Control      SG_COMMA)      all_ones,     u32(VK_CONTROL)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::LeftShift    SG_COMMA)      all_ones,     u32(all_ones)    },
        { SG_CODE_FOR_ASSERT(KeyboardKey::LeftControl  SG_COMMA)      all_ones,     u32(all_ones)    },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Menu         SG_COMMA)      all_ones,     u32(VK_MENU)     },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Pause        SG_COMMA)      all_ones,     u32(VK_PAUSE)    },
        { SG_CODE_FOR_ASSERT(KeyboardKey::CapsLock     SG_COMMA)      all_ones,     u32(VK_CAPITAL)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Escape       SG_COMMA)      all_ones,     u32(VK_ESCAPE)   },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Space        SG_COMMA)      all_ones,     u32(VK_SPACE)    },
        { SG_CODE_FOR_ASSERT(KeyboardKey::PageUp       SG_COMMA)      all_ones,     u32(VK_PRIOR)    },
        { SG_CODE_FOR_ASSERT(KeyboardKey::PageNext     SG_COMMA)      all_ones,     u32(VK_NEXT)     },
        { SG_CODE_FOR_ASSERT(KeyboardKey::End          SG_COMMA)      all_ones,     u32(VK_END)      },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Home         SG_COMMA)      all_ones,     u32(VK_HOME)     },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Left         SG_COMMA)      all_ones,     u32(VK_LEFT)     },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Up           SG_COMMA)      all_ones,     u32(VK_UP)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Right        SG_COMMA)      all_ones,     u32(VK_RIGHT)    },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Down         SG_COMMA)      all_ones,     u32(VK_DOWN)     },
        { SG_CODE_FOR_ASSERT(KeyboardKey::PrintScreen  SG_COMMA)      all_ones,     u32(VK_SNAPSHOT) },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Insert       SG_COMMA)      all_ones,     u32(VK_INSERT)   },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Delete       SG_COMMA)      all_ones,     u32(VK_DELETE)   },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_0        SG_COMMA)      all_ones,     u32('0')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_1        SG_COMMA)      all_ones,     u32('1')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_2        SG_COMMA)      all_ones,     u32('2')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_3        SG_COMMA)      all_ones,     u32('3')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_4        SG_COMMA)      all_ones,     u32('4')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_5        SG_COMMA)      all_ones,     u32('5')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_6        SG_COMMA)      all_ones,     u32('6')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_7        SG_COMMA)      all_ones,     u32('7')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_8        SG_COMMA)      all_ones,     u32('8')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_9        SG_COMMA)      all_ones,     u32('9')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_A        SG_COMMA)      all_ones,     u32('A')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_B        SG_COMMA)      all_ones,     u32('B')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_C        SG_COMMA)      all_ones,     u32('C')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_D        SG_COMMA)      all_ones,     u32('D')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_E        SG_COMMA)      all_ones,     u32('E')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_F        SG_COMMA)      all_ones,     u32('F')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_G        SG_COMMA)      all_ones,     u32('G')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_H        SG_COMMA)      all_ones,     u32('H')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_I        SG_COMMA)      all_ones,     u32('I')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_J        SG_COMMA)      all_ones,     u32('J')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_K        SG_COMMA)      all_ones,     u32('K')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_L        SG_COMMA)      all_ones,     u32('L')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_M        SG_COMMA)      all_ones,     u32('M')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_N        SG_COMMA)      all_ones,     u32('N')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_O        SG_COMMA)      all_ones,     u32('O')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_P        SG_COMMA)      all_ones,     u32('P')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_Q        SG_COMMA)      all_ones,     u32('Q')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_R        SG_COMMA)      all_ones,     u32('R')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_S        SG_COMMA)      all_ones,     u32('S')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_T        SG_COMMA)      all_ones,     u32('T')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_U        SG_COMMA)      all_ones,     u32('U')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_V        SG_COMMA)      all_ones,     u32('V')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_W        SG_COMMA)      all_ones,     u32('W')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_X        SG_COMMA)      all_ones,     u32('X')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_Y        SG_COMMA)      all_ones,     u32('Y')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Key_Z        SG_COMMA)      all_ones,     u32('Z')         },
        { SG_CODE_FOR_ASSERT(KeyboardKey::LeftWin      SG_COMMA)      all_ones,     u32(VK_LWIN)     },
        { SG_CODE_FOR_ASSERT(KeyboardKey::RightWin     SG_COMMA)      all_ones,     u32(VK_RWIN)     },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Apps         SG_COMMA)      all_ones,     u32(VK_APPS)     },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_0     SG_COMMA)      all_ones,     u32(VK_NUMPAD0)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_1     SG_COMMA)      all_ones,     u32(VK_NUMPAD1)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_2     SG_COMMA)      all_ones,     u32(VK_NUMPAD2)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_3     SG_COMMA)      all_ones,     u32(VK_NUMPAD3)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_4     SG_COMMA)      all_ones,     u32(VK_NUMPAD4)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_5     SG_COMMA)      all_ones,     u32(VK_NUMPAD5)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_6     SG_COMMA)      all_ones,     u32(VK_NUMPAD6)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_7     SG_COMMA)      all_ones,     u32(VK_NUMPAD7)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_8     SG_COMMA)      all_ones,     u32(VK_NUMPAD8)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Numpad_9     SG_COMMA)      all_ones,     u32(VK_NUMPAD9)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Multiply     SG_COMMA)      all_ones,     u32(VK_MULTIPLY) },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Add          SG_COMMA)      all_ones,     u32(VK_ADD)      },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Subtract     SG_COMMA)      all_ones,     u32(VK_SUBTRACT) },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Decimal      SG_COMMA)      all_ones,     u32(VK_DECIMAL)  },
        { SG_CODE_FOR_ASSERT(KeyboardKey::Divide       SG_COMMA)      all_ones,     u32(VK_DIVIDE)   },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F1           SG_COMMA)      all_ones,     u32(VK_F1)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F2           SG_COMMA)      all_ones,     u32(VK_F2)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F3           SG_COMMA)      all_ones,     u32(VK_F3)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F4           SG_COMMA)      all_ones,     u32(VK_F4)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F5           SG_COMMA)      all_ones,     u32(VK_F5)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F6           SG_COMMA)      all_ones,     u32(VK_F6)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F7           SG_COMMA)      all_ones,     u32(VK_F7)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F8           SG_COMMA)      all_ones,     u32(VK_F8)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F9           SG_COMMA)      all_ones,     u32(VK_F9)       },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F10          SG_COMMA)      all_ones,     u32(VK_F10)      },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F11          SG_COMMA)      all_ones,     u32(VK_F11)      },
        { SG_CODE_FOR_ASSERT(KeyboardKey::F12          SG_COMMA)      all_ones,     u32(VK_F12)      },
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
#if SG_PLATFORM_IS_WIN
    SG_ASSERT(static_cast<size_t>(iKey) < SG_ARRAYSIZE(keyboardKeyTraits));
    KeyboardKeyTraits const& traits = keyboardKeyTraits[static_cast<size_t>(iKey)];
    SG_ASSERT(traits.key == iKey);
    switch(iLayout)
    {
    case KeyboardLayout::QWERTY:
        {
            SG_ASSERT_MSG(-1 != traits.scanCodeForQWERTY, "This key is not supported in this layout");
            m_code = traits.scanCodeForQWERTY;
            m_isScanCodeElseVirtualKey = true;
        }
        break;
    case KeyboardLayout::UserLayout:
        {
            SG_ASSERT_MSG(-1 != traits.virtualKey, "This key is not supported in this layout");
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
bool KeyboardEntryAdapter::DoesMatch(UserInputEvent const& iEvent) const
{
    if(UserInputDeviceType::Keyboard != iEvent.DeviceType())
        return false;
    if(UserInputEventType::OffToOn != iEvent.EventType()
       && UserInputEventType::OnRepeat != iEvent.EventType()
       && UserInputEventType::OnToOff != iEvent.EventType())
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
}
}
