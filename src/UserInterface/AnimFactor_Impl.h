#ifndef UserInterface_AnimFactor_Include_Impl
#error "this file must be used only by AnimFactor.h"
#endif

#include <Core/For.h>
#include <Math/NumericalUtils.h>
#include <algorithm>

namespace sg {
namespace ui {
namespace internal {
//=============================================================================
template <typename Derived, AnimFactorFlag Flag>
AnimFactorBase<Derived, Flag>::AnimFactorBase(float iDuration)
    : m_durationInTicks(1)
    , m_beginTickWhenPlay(0)
    , m_backward(false)
    , m_pause(true)
#if SG_ENABLE_ASSERT
    , m_guard(0xCAFEFADE)
#endif
{
    static_assert(std::is_same<TimeServer const*, decltype(((Derived const*)0)->GetTimeServerImpl())>::value, "Derived must implement: TimeServer const* GetTimeServer();");
    ChangeDuration(iDuration);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
AnimFactorBase<Derived, Flag>::~AnimFactorBase()
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0xCAFEFADE == m_guard);
    m_guard = 0;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::Play()
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0xCAFEFADE == m_guard);
#endif
    if(m_pause)
    {
        TimeServer const* timeServer = GetTimeServer();
        u32 const tick = timeServer->Tick();
        m_beginTickWhenPlay = tick - m_deltaTickWhenPause;
        m_pause = false;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::PlayBackward()
{
    Backward();
    Play();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::PlayForward()
{
    Forward();
    Play();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::Pause()
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0xCAFEFADE == m_guard);
#endif
    if(!m_pause)
    {
        // Note: Calling Derived::GetValueImpl is necessary for events to be
        // activated if needed.
        u32 boundedDeltaTick;
        u32 cycleCount;
        float const t = static_cast<Derived*>(this)->GetValueImpl(boundedDeltaTick, cycleCount);
        PauseImpl(boundedDeltaTick);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::PauseImpl(u32 boundedDeltaTick)
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0xCAFEFADE == m_guard);
#endif
    SG_ASSERT(!m_pause);
    m_deltaTickWhenPause = boundedDeltaTick;
    m_pause = true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::Backward()
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0xCAFEFADE == m_guard);
#endif
    if(!m_backward)
        Reverse();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::Forward()
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0xCAFEFADE == m_guard);
#endif
    if(m_backward)
        Reverse();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::Reverse()
{
    bool const prevPause = m_pause;
    Pause();
    SG_ASSERT(m_deltaTickWhenPause <= m_durationInTicks);
    m_deltaTickWhenPause = m_durationInTicks - m_deltaTickWhenPause;
    SG_ASSERT(m_deltaTickWhenPause <= m_durationInTicks);
    m_backward = !m_backward;
    if(!prevPause) Play();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::ChangeDuration(float iDuration)
{
    SG_ASSERT(0 < iDuration);
    bool const prevPause = m_pause;
    Pause();
    TimeServer const* timeServer = GetTimeServer();
    u32 const frequency = timeServer->Frequency();
    u32 const newDurationInTicks = roundi(frequency * iDuration);
    SG_ASSERT(0 < newDurationInTicks);
    m_deltaTickWhenPause = (m_deltaTickWhenPause * newDurationInTicks + m_durationInTicks/2) / m_durationInTicks;
    m_durationInTicks = newDurationInTicks;
    if(!prevPause) Play();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorBase<Derived, Flag>::SetValueImpl(float iValue)
{
    SG_ASSERT(0 <= iValue);
    SG_ASSERT(1 >= iValue);
    bool const prevPause = m_pause;
    bool const prevBackward = m_backward;
    Pause();
    Forward();
    m_deltaTickWhenPause = roundi(iValue * m_durationInTicks);
    if(prevBackward) Backward();
    if(!prevPause) Play();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
float AnimFactorBase<Derived, Flag>::GetValue()
{
    u32 boundedDeltaTick;
    u32 cycleCount;
    return static_cast<Derived*>(this)->GetValueImpl(boundedDeltaTick, cycleCount);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
float AnimFactorBase<Derived, Flag>::GetValueImpl(u32& oBoundedDeltaTick, u32& oCycleCount)
{
    TimeServer const* timeServer = GetTimeServer();
    u32 const frequency = timeServer->Frequency();
    u32 const tick = timeServer->Tick();
    u32 boundedDeltaTick = 0;
    if(m_pause)
    {
        boundedDeltaTick = m_deltaTickWhenPause;
        SG_ASSERT(m_deltaTickWhenPause <= m_durationInTicks);
    }
    else
    {
        SG_ASSERT(i32(tick-m_beginTickWhenPlay) >= 0);
        u32 const deltaTick = m_pause ? m_deltaTickWhenPause : tick - m_beginTickWhenPlay;
        switch(Flag)
        {
        case AnimFactorFlag::Clamp :
            boundedDeltaTick = std::min(deltaTick, m_durationInTicks);
            m_beginTickWhenPlay = tick - boundedDeltaTick;
            oCycleCount = 0;
            break;
        case AnimFactorFlag::Cyclic :
            boundedDeltaTick = deltaTick % m_durationInTicks;
            oCycleCount = deltaTick / m_durationInTicks;
            m_beginTickWhenPlay += oCycleCount * m_durationInTicks;
            break;
        default:
            SG_ASSUME_NOT_REACHED();
        }
    }
    oBoundedDeltaTick = boundedDeltaTick;
    u32 const ticksTo0 = m_backward ? m_durationInTicks - boundedDeltaTick : boundedDeltaTick;
    SG_ASSERT(ticksTo0 <= m_durationInTicks);
    float const t = float(ticksTo0) / float(m_durationInTicks);
    SG_ASSERT(0 <= t);
    SG_ASSERT(1 >= t);
    return t;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
TimeServer const* AnimFactorBase<Derived, Flag>::GetTimeServer() const
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0xCAFEFADE == m_guard);
#endif
    return static_cast<Derived const*>(this)->GetTimeServerImpl();
}
//=============================================================================
template <typename Derived, AnimFactorFlag Flag>
AnimFactorWithEvents<Derived, Flag>::AnimFactorWithEvents(float iDuration)
    : parent_type(iDuration)
    , m_keyEvents()
    , m_cursor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorWithEvents<Derived, Flag>::SetValue(float iValue)
{
    parent_type::SetValueImpl(iValue);

    m_cursor = nullptr;
    for(auto& it : m_keyEvents)
    {
        if(it.key >= iValue)
        {
            // As the behaviour between forward and backward is not symmetric
            // when the key is equal to current value (when going forward, the
            // event will be activated next time update, but not when going
            // backward), this special case is forbidden.
            // If you need to have this case working, one simple solution is to
            // call Forward(), then SetValue(), then Backward(). However, note
            // that the event won't be activated next update.
            SG_ASSERT_MSG(!m_backward || it.key != iValue || (static_cast<size_t>(it.flags) & static_cast<size_t>(AnimFactorEventFlag::Backward)) == 0, "This case is forbidden.");
            m_cursor = &it;
            break;
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
template <AnimFactorEventFlag Direction>
void AnimFactorWithEvents<Derived, Flag>::HandleKeyEvent(KeyEvent* iKeyEvent, u32 iBoundedDeltaTick, float& ioValue, bool& oPauseEncountered)
{
    static_assert(Direction == AnimFactorEventFlag::Forward || Direction == AnimFactorEventFlag::Backward, "Direction must be Forward or Backward");
    if((static_cast<size_t>(iKeyEvent->flags) & static_cast<size_t>(Direction)) != 0)
    {
        iKeyEvent->listener->VirtualOnAnimFactorKey(this, iKeyEvent->key, iKeyEvent->flags);
        if((static_cast<size_t>(iKeyEvent->flags) & static_cast<size_t>(AnimFactorEventFlag::StopAtKey)) != 0)
        {
            oPauseEncountered = true;
            PauseImpl(iBoundedDeltaTick);
            ioValue = iKeyEvent->key;
            parent_type::SetValueImpl(ioValue);
        }
        if((static_cast<size_t>(iKeyEvent->flags) & static_cast<size_t>(AnimFactorEventFlag::OneTime)) != 0)
        {
            KeyEvent* toRemove = iKeyEvent;
            iKeyEvent = nullptr;
            m_keyEvents.Remove(toRemove);
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
float AnimFactorWithEvents<Derived, Flag>::GetValueImpl(u32& oBoundedDeltaTick, u32& oCycleCount)
{
    float value = parent_type::GetValueImpl(oBoundedDeltaTick, oCycleCount);
    if(!m_pause)
    {
        bool pauseEncountered = false;
        if(m_backward)
        {
            if(SG_CONSTANT_CONDITION(Flag == AnimFactorFlag::Cyclic))
            {
                SG_ASSERT_MSG(oCycleCount <= 1, "Is it wise to have an anim factor cycling that much?");
                for_range_0(size_t, i, oCycleCount)
                {
                    safeptr<KeyEvent> cursor = (nullptr == m_cursor) ? m_keyEvents.Back() : m_keyEvents.Before(m_cursor.get());
                    while(nullptr != cursor && !pauseEncountered)
                    {
                        KeyEvent* keyEvent = cursor.get();
                        cursor = m_keyEvents.Before(cursor.get());
                        HandleKeyEvent<AnimFactorEventFlag::Backward>(keyEvent, oBoundedDeltaTick, value, pauseEncountered);
                    }
                    m_cursor = nullptr;
                }
            }
            safeptr<KeyEvent> cursor = (nullptr == m_cursor) ? m_keyEvents.Back() : m_keyEvents.Before(m_cursor.get());
            while(nullptr != cursor && !pauseEncountered)
            {
                if(cursor->key >= value)
                {
                    KeyEvent* keyEvent = cursor.get();
                    cursor = m_keyEvents.Before(cursor.get());
                    HandleKeyEvent<AnimFactorEventFlag::Backward>(keyEvent, oBoundedDeltaTick, value, pauseEncountered);
                }
                else
                {
                    break;
                }
            }
            if(nullptr == cursor)
                m_cursor = m_keyEvents.Front();
            else
                m_cursor = m_keyEvents.After(cursor.get());
        }
        else
        {
            if(SG_CONSTANT_CONDITION(Flag == AnimFactorFlag::Cyclic))
            {
                SG_ASSERT_MSG(oCycleCount <= 1, "Is it wise to have an anim factor cycling that much?");
                for_range_0(size_t, i, oCycleCount)
                {
                    safeptr<KeyEvent> cursor = m_cursor;
                    while(nullptr != cursor && !pauseEncountered)
                    {
                        KeyEvent* keyEvent = cursor.get();
                        cursor = m_keyEvents.After(cursor.get());
                        HandleKeyEvent<AnimFactorEventFlag::Forward>(keyEvent, oBoundedDeltaTick, value, pauseEncountered);
                    }
                    m_cursor = m_keyEvents.Front();
                }
            }
            safeptr<KeyEvent> cursor = m_cursor;
            while(nullptr != cursor && !pauseEncountered)
            {
                if(cursor->key <= value)
                {
                    KeyEvent* keyEvent = cursor.get();
                    cursor = m_keyEvents.After(cursor.get());
                    HandleKeyEvent<AnimFactorEventFlag::Forward>(keyEvent, oBoundedDeltaTick, value, pauseEncountered);
                }
                else
                {
                    break;
                }
            }
            m_cursor = cursor;
        }
    }
    return value;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorWithEvents<Derived, Flag>::AddEvent(float iKey, IAnimFactorEventListener* iListener, AnimFactorEventFlag iFlags)
{
    float const animValue = GetValue();
    // As the behaviour between forward and backward is not symmetric when the
    // key is equal to current value (when going forward, the event will be
    // activated next time update, but not when going backward), this special
    // case is forbidden.
    SG_ASSERT_MSG(!m_backward || animValue != iKey || (static_cast<size_t>(iFlags) & static_cast<size_t>(AnimFactorEventFlag::Backward)) == 0, "This case is forbidden.");
    KeyEvent* keyEvent = new KeyEvent(iKey, iListener, iFlags);
    if(m_keyEvents.Empty())
    {
        m_keyEvents.PushBack(keyEvent);
        SG_ASSERT(nullptr == m_cursor);
        if(iKey >= animValue)
            m_cursor = keyEvent;
    }
    else
    {
        for(auto& it : m_keyEvents)
        {
            if(it.key > iKey)
            {
                m_keyEvents.InsertBefore(&it, keyEvent);
                if((m_cursor == &it) && (iKey >= animValue))
                        m_cursor = keyEvent;
                break;
            }
        }
    }
    if(!m_keyEvents.Contains(keyEvent))
    {
        m_keyEvents.InsertAfter(m_keyEvents.Back(), keyEvent);
        if((nullptr == m_cursor) && (iKey >= animValue))
            m_cursor = keyEvent;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorWithEvents<Derived, Flag>::RemoveEvent(float iKey, IAnimFactorEventListener* iListener, AnimFactorEventFlag iFlags)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
void AnimFactorWithEvents<Derived, Flag>::RemoveListener(IAnimFactorEventListener* iListener)
{
}
//=============================================================================
}
}
}
