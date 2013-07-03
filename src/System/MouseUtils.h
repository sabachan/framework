#ifndef System_MouseUtils_H
#define System_MouseUtils_H

#include <Core/Platform.h>
#include <Core/IntTypes.h>

namespace sg {
namespace system {
//=============================================================================
enum class MouseEntry : u16
{
    Position    = 0x0000,
    Button0     = 0x0001,
    Button1     = 0x0002,
    Button2     = 0x0004,
    Button3     = 0x0008,
    Button4     = 0x0010,
    Shift       = 0x0020,
    Control     = 0x0040,
    Wheel       = 0x0080,
};
//=============================================================================
struct MouseState
{
public:
    static u8 const MOUSE_BUTTONS_MASK = 0x1F;
public:
    MouseState() : data(0) {}
    explicit MouseState(u64 iData) : data(iData) {}
    i16vec2 Position() const { return i16vec2(data & 0xFFFF, (data >> 16) & 0xFFFF); }
    u8 GetButtonsDown() const { u8 const flags = (data >> 32) & 0xFF; return flags; }
    u8 GetMouseButtonsDown() const { u8 const flags = (data >> 32) & MOUSE_BUTTONS_MASK; return flags; }
    bool AnyMouseButtonDown() const { u8 const flags = (data >> 32) & MOUSE_BUTTONS_MASK; return 0 != flags; }
    bool StrictlyMoreThanOneMouseButtonDown() const { u8 const flags = (data >> 32) & MOUSE_BUTTONS_MASK; return 0 != (flags & (flags-1)); }
    bool NoMouseButtonDown() const { u8 const flags = (data >> 32) & MOUSE_BUTTONS_MASK; return 0 == flags; }
    bool IsButtonDown(MouseEntry iButton) const;
    void SetPosition(i16vec2 const& iPosition) { data |= ((u64(iPosition.x()) & 0xFFFF) | ((u64(iPosition.y()) & 0xFFFF) << 16)); }
    void SetButtonsDown(u8 iButtonsDown) { data |= (u64(iButtonsDown) << 32); }
    u64 Data() const { return data; }
private:
    u64 data;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline bool MouseState::IsButtonDown(MouseEntry iButton) const
{
    SG_ASSERT(MouseEntry::Button0 == iButton
           || MouseEntry::Button1 == iButton
           || MouseEntry::Button2 == iButton
           || MouseEntry::Button3 == iButton
           || MouseEntry::Button4 == iButton
           || MouseEntry::Shift   == iButton
           || MouseEntry::Control == iButton);
    u8 const buttonFlag = static_cast<u8>(iButton);
    SG_ASSERT(0 != buttonFlag);
    SG_ASSERT((buttonFlag & (buttonFlag - 1)) == 0);
    return 0 != ((data >> 32) & buttonFlag);
}
//=============================================================================
struct MouseButtonAdditionalData
{
public:
    MouseButtonAdditionalData() : data(0) {}
    explicit MouseButtonAdditionalData(u32 iData) : data(iData) {}
    bool IsDoubleClick() const { return 0 != (data & 0x1); }
    void SetDoubleClick(bool iValue) { if(iValue) { data |= 0x1; } else { data &= ~0x1; } }
    u32 Data() const { return data; }
private:
    u32 data;
};
//=============================================================================
struct MouseWheelAdditionalData
{
public:
    MouseWheelAdditionalData() : data(0) {}
    explicit MouseWheelAdditionalData(u32 iData) : data(iData) {}
    u32 Data() const { return data; }
    void SetWheelDeltaFP16(i32 iWheelRotation) { data = iWheelRotation; }
    i32 WheelDeltaFP16() const { return i32(data); }
private:
    u32 data;
};
//=============================================================================
}
}

#endif
