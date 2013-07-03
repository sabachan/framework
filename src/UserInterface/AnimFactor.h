#ifndef UserInterface_AnimFactor_H
#define UserInterface_AnimFactor_H

#include <Core/IntrusiveList.h>
#include <Core/TimeServer.h>
#include <vector>

namespace sg {
namespace ui {
//=============================================================================
enum class AnimFactorEventFlag
{
    Forward = 0x1,
    Backward = 0x2,
    OneTime = 0x4,
    StopAtKey = 0x8,

    Forward_OneTime = Forward | OneTime,
    Forward_StopAtKey = Forward | StopAtKey,
    Forward_OneTime_StopAtKey = Forward | OneTime | StopAtKey,

    Backward_OneTime = Backward | OneTime,
    Backward_StopAtKey = Backward | StopAtKey,
    Backward_OneTime_StopAtKey = Backward | OneTime | StopAtKey,

    Forward_Backward = Forward | Backward,
    Forward_Backward_OneTime = Forward_Backward | OneTime,
    Forward_Backward_StopAtKey = Forward_Backward | StopAtKey,
    Forward_Backward_OneTime_StopAtKey = Forward_Backward | OneTime | StopAtKey,
};
//=============================================================================
// Note: Un listener n'a pas le droit de changer l'état de son AnimFactor dans
// la fonction VirtualOnAnimFactorKey.
class IAnimFactorEventListener : public SafeCountable
{
public:
    virtual void VirtualOnAnimFactorKey(void* iAnimFactor, float iKey, AnimFactorEventFlag iFlags) = 0;
};
//=============================================================================
namespace internal {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
enum class AnimFactorFlag
{
    Clamp,
    Cyclic,
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
class AnimFactorBase
{
public:
    AnimFactorBase(float iDuration);
    ~AnimFactorBase();
    void Play();
    void PlayBackward();
    void PlayForward();
    void Pause();
    void Backward();
    void Forward();
    void Reverse();
    void ChangeDuration(float iDuration);
    bool IsPaused() const { return m_pause; }
    bool IsBackward() const { return m_backward; }
    float GetValue();
protected:
    void SetValueImpl(float iValue);
    float GetValueImpl(u32& oDeltaTicks, u32& oCycleCount);
    void PauseImpl(u32 boundedDeltaTick);
private:
    TimeServer const* GetTimeServer() const;
private:
    template <typename Derived, AnimFactorFlag Flag> friend class AnimFactorWithEvents;
private:
    u32 m_durationInTicks;
    union { u32 m_beginTickWhenPlay; u32 m_deltaTickWhenPause; };
    bool m_backward;
    bool m_pause;
#if SG_ENABLE_ASSERT
    u32 m_guard;
#endif
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
class AnimFactor : public AnimFactorBase<Derived, Flag>
{
    typedef AnimFactorBase<Derived, Flag> parent_type;
public:
    AnimFactor(float iDuration) : AnimFactorBase(iDuration) {}
    void SetValue(float iValue) { parent_type::SetValueImpl(iValue); }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename Derived, AnimFactorFlag Flag>
class AnimFactorWithEvents : public AnimFactorBase<Derived, Flag>
{
    typedef AnimFactorBase<Derived, Flag> parent_type;
public:
    AnimFactorWithEvents(float iDuration);
    void SetValue(float iValue);
    void AddEvent(float iKey, IAnimFactorEventListener* iListener, AnimFactorEventFlag iFlags = AnimFactorEventFlag::Forward | AnimFactorEventFlag::Backward);
    void RemoveEvent(float iKey, IAnimFactorEventListener* iListener, AnimFactorEventFlag iFlags);
    void RemoveListener(IAnimFactorEventListener* iListener);
private:
    struct KeyEvent : public RefCountable
                    , public IntrusiveListRefCountableElem<KeyEvent>
    {
    public:
        float key;
        safeptr<IAnimFactorEventListener> listener;
        AnimFactorEventFlag flags;
    public:
        KeyEvent(float iKey, IAnimFactorEventListener* iListener, AnimFactorEventFlag iFlags)
            : key(iKey)
            , listener(iListener)
            , flags(iFlags)
        {
        }
    };
private:
    friend class parent_type;
    float GetValueImpl(u32& oBoundedDeltaTick, u32& oCycleCount);
    template <AnimFactorEventFlag Direction>
    void HandleKeyEvent(KeyEvent* iKeyEvent, u32 iBoundedDeltaTick, float& ioValue, bool& oPauseEncountered);
private:
    IntrusiveList<KeyEvent> m_keyEvents;
    safeptr<KeyEvent> m_cursor;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
template <typename TimeServerSingeton>
class AnimFactorOnTimeServer : public internal::AnimFactor<AnimFactorOnTimeServer<TimeServerSingeton>, internal::AnimFactorFlag::Clamp>
{
    typedef internal::AnimFactor<AnimFactorOnTimeServer<TimeServerSingeton>, internal::AnimFactorFlag::Clamp> parent_type;
public:
    AnimFactorOnTimeServer(float iDuration) : parent_type(iDuration) {}
    TimeServer const* GetTimeServerImpl() const { TimeServer const* timeServer = TimeServerSingeton::GetIFP(); SG_ASSERT(nullptr != timeServer); return timeServer; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename TimeServerSingeton>
class CyclicAnimFactorOnTimeServer : public internal::AnimFactor<CyclicAnimFactorOnTimeServer<TimeServerSingeton>, internal::AnimFactorFlag::Cyclic>
{
    typedef internal::AnimFactor<CyclicAnimFactorOnTimeServer<TimeServerSingeton>, internal::AnimFactorFlag::Cyclic> parent_type;
public:
    CyclicAnimFactorOnTimeServer(float iDuration) : parent_type(iDuration) {}
    TimeServer const* GetTimeServerImpl() const { TimeServer const* timeServer = TimeServerSingeton::GetIFP(); SG_ASSERT(nullptr != timeServer); return timeServer; }
};
//=============================================================================
template <typename TimeServerSingeton>
class AnimFactorWithEventsOnTimeServer : public internal::AnimFactorWithEvents<AnimFactorWithEventsOnTimeServer<TimeServerSingeton>, internal::AnimFactorFlag::Clamp>
{
    typedef internal::AnimFactorWithEvents<AnimFactorWithEventsOnTimeServer<TimeServerSingeton>, internal::AnimFactorFlag::Clamp> parent_type;
public:
    AnimFactorWithEventsOnTimeServer(float iDuration) : parent_type(iDuration) {}
    TimeServer const* GetTimeServerImpl() const { TimeServer const* timeServer = TimeServerSingeton::GetIFP(); SG_ASSERT(nullptr != timeServer); return timeServer; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename TimeServerSingeton>
class CyclicAnimFactorWithEventsOnTimeServer : public internal::AnimFactorWithEvents<CyclicAnimFactorWithEventsOnTimeServer<TimeServerSingeton>, internal::AnimFactorFlag::Cyclic>
{
    typedef internal::AnimFactorWithEvents<CyclicAnimFactorWithEventsOnTimeServer<TimeServerSingeton>, internal::AnimFactorFlag::Cyclic> parent_type;
public:
    CyclicAnimFactorWithEventsOnTimeServer(float iDuration) : parent_type(iDuration) {}
    TimeServer const* GetTimeServerImpl() const { TimeServer const* timeServer = TimeServerSingeton::GetIFP(); SG_ASSERT(nullptr != timeServer); return timeServer; }
};
//=============================================================================
template <internal::AnimFactorFlag Flag>
class AnimFactorBase : public internal::AnimFactor<AnimFactorBase<Flag>, Flag>
{
    typedef internal::AnimFactor<AnimFactorBase<Flag>, Flag> parent_type;
public:
    AnimFactorBase(float iDuration, TimeServer const* iTimeServer) : parent_type(iDuration), m_timeServer(iTimeServer) { SG_ASSERT(nullptr != iTimeServer); }
    TimeServer const* GetTimeServerImpl() const { SG_ASSERT(nullptr != m_timeServer); return m_timeServer.get(); }
private:
    safeptr<TimeServer const> m_timeServer;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
typedef AnimFactorBase<internal::AnimFactorFlag::Clamp> AnimFactor;
typedef AnimFactorBase<internal::AnimFactorFlag::Cyclic> CyclicAnimFactor;
//=============================================================================
template <internal::AnimFactorFlag Flag>
class AnimFactorWithEventsBase : public internal::AnimFactorWithEvents<AnimFactorWithEventsBase<Flag>, Flag>
{
    typedef internal::AnimFactorWithEvents<AnimFactorWithEventsBase<Flag>, Flag> parent_type;
public:
    AnimFactorWithEventsBase(float iDuration, TimeServer const* iTimeServer) : parent_type(iDuration), m_timeServer(iTimeServer) {}
    TimeServer const* GetTimeServerImpl() const { SG_ASSERT(nullptr != m_timeServer); return m_timeServer.get(); }
private:
    safeptr<TimeServer const> m_timeServer;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
typedef AnimFactorWithEventsBase<internal::AnimFactorFlag::Cyclic> AnimFactorWithEvents;
typedef AnimFactorWithEventsBase<internal::AnimFactorFlag::Cyclic> CyclicAnimFactorWithEvents;
//=============================================================================
}
}

#define UserInterface_AnimFactor_Include_Impl
#include "AnimFactor_Impl.h"
#undef UserInterface_AnimFactor_Include_Impl

#endif
