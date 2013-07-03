#include "stdafx.h"

#include "UserInputEvent.h"
#include "Window.h"

namespace sg {
namespace system {
//=============================================================================
UserInputEvent::UserInputEvent(reset_all_t)
    : m_masked(false)
    , m_premasked(false)
    , m_deviceType(UserInputDeviceType::None)
    , m_deviceId(all_ones)
    , m_type(UserInputEventType::ResetAll)
    , m_entryId(all_ones)
    , m_additionalData(0)
    , m_state(0)
    , m_window(nullptr)
#if SG_ENABLE_ASSERT
    , m_index(0)
    , m_batchIndex(0)
#endif
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
UserInputEvent::UserInputEvent(Window const* iWindow, size_t iIndex, size_t iBatchIndex)
#else
UserInputEvent::UserInputEvent(Window const* iWindow)
#endif
    : m_masked(false)
    , m_premasked(false)
    , m_deviceType(UserInputDeviceType::None)
    , m_deviceId(all_ones)
    , m_type(UserInputEventType::None)
    , m_entryId(all_ones)
    , m_additionalData(0)
    , m_state(0)
    , m_window(iWindow)
#if SG_ENABLE_ASSERT
    , m_index(iIndex)
    , m_batchIndex(iBatchIndex)
#endif
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
UserInputEvent::~UserInputEvent()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UserInputEvent::SetDevice(UserInputDeviceType iDeviceType, u8 iDeviceId)
{
    SG_ASSERT(UserInputDeviceType::None == m_deviceType);
    SG_ASSERT(UserInputDeviceType::None != iDeviceType);
    m_deviceType = iDeviceType;
    m_deviceId = iDeviceId;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UserInputEvent::SetEntry(UserInputEventType iEventType, u32 iEntryId)
{
    SG_ASSERT(UserInputEventType::None == m_type);
    SG_ASSERT(UserInputEventType::None != iEventType);
    SG_ASSERT_MSG(-1 != iEntryId, "-1 is reserved for invalid entry");
    m_type = iEventType;
    m_entryId = iEntryId;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UserInputEvent::SetAdditionalData(u32 iData)
{
    SG_ASSERT(0 == m_additionalData);
    m_additionalData = iData;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void UserInputEvent::SetState(u64 iState)
{
    SG_ASSERT(0 == m_state);
    m_state = iState;
}
//=============================================================================
}
}
