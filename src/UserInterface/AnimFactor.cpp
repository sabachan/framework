#include "stdafx.h"

#include "AnimFactor.h"

#include <Core/Singleton.h>
#include <Core/TimeServer.h>
#include <Core/TestFramework.h>

#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace ui {
//=============================================================================
namespace {
    class TimeServerForTest : public TimeServer
                            , public Singleton<TimeServerForTest>
    {
        PARENT_SAFE_COUNTABLE(TimeServer)
    public:
        TimeServerForTest() : TimeServer(1 << 8) {}
    };
    class EventListenerForTest : public IAnimFactorEventListener
    {
    public:
        EventListenerForTest() : m_hitCount(0) {}
        virtual void VirtualOnAnimFactorKey(void* iAnimFactor, float iKey, AnimFactorEventFlag iFlags) override { SG_UNUSED((iAnimFactor,iKey,iFlags)); ++m_hitCount; }
        size_t HitCount() const { return m_hitCount; }
        void Reset() { m_hitCount = 0; }
    private:
        size_t m_hitCount;
    };
}
//=============================================================================
SG_TEST((sg, ui), AnimFactor, (UserInterface, quick))
{
    TimeServerForTest timeServer;

    for_range(size_t, i, 0, 2)
    {
        timeServer.SetTick(u32(i * -467907));
        AnimFactorOnTimeServer<TimeServerForTest> animFactor(8.f);
        animFactor.Play();
        float t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        animFactor.Pause();
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        animFactor.Play();
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.75f == t);
        animFactor.Reverse();
        t = animFactor.GetValue();
        SG_ASSERT(0.75f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        animFactor.Forward();
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.75f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(1.f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(1.f == t);
        animFactor.Backward();
        t = animFactor.GetValue();
        SG_ASSERT(1.f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.75f == t);
        animFactor.SetValue(0.25f);
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        animFactor.Pause();
        t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        animFactor.Play();
        t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        animFactor.Forward();
        t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
    }

    {
        timeServer.SetTick(0);
        CyclicAnimFactorOnTimeServer<TimeServerForTest> animFactor(8.f);
        animFactor.Play();
        float t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.75f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.f == t || 1.f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        animFactor.Backward();
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.f == t || 1.f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.75f == t);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
    }

    {
        timeServer.SetTick(0);
        EventListenerForTest a;
        EventListenerForTest b;
        EventListenerForTest c;
        EventListenerForTest d;
        EventListenerForTest e;
        EventListenerForTest f;
        AnimFactorWithEventsOnTimeServer<TimeServerForTest> animFactor(8.f);
        animFactor.AddEvent(0.00f, &a, AnimFactorEventFlag::Forward_Backward);
        animFactor.AddEvent(0.10f, &a, AnimFactorEventFlag::Forward);
        animFactor.AddEvent(0.20f, &a, AnimFactorEventFlag::Backward);
        animFactor.AddEvent(0.30f, &a, AnimFactorEventFlag::Forward);
        animFactor.AddEvent(0.40f, &a, AnimFactorEventFlag::Backward);
        animFactor.AddEvent(0.50f, &a, AnimFactorEventFlag::Forward_Backward);
        animFactor.AddEvent(0.60f, &a, AnimFactorEventFlag::Backward);
        animFactor.AddEvent(0.70f, &a, AnimFactorEventFlag::Forward);
        animFactor.AddEvent(0.80f, &a, AnimFactorEventFlag::Backward);
        animFactor.AddEvent(0.90f, &a, AnimFactorEventFlag::Forward);
        animFactor.AddEvent(1.00f, &a, AnimFactorEventFlag::Forward_Backward);

        animFactor.AddEvent(0.375f, &b, AnimFactorEventFlag::Forward_StopAtKey);
        animFactor.AddEvent(0.375f, &c, AnimFactorEventFlag::Forward_StopAtKey);
        animFactor.AddEvent(0.375f, &d, AnimFactorEventFlag::Forward_StopAtKey);

        animFactor.AddEvent(0.625f, &e, AnimFactorEventFlag::Backward_StopAtKey);
        animFactor.AddEvent(0.875f, &f, AnimFactorEventFlag::Forward_OneTime);

        animFactor.Play();
        float t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        SG_ASSERT(1 == a.HitCount());
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        SG_ASSERT(2 == a.HitCount());
        SG_ASSERT(0 == b.HitCount());
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.375f == t);
        SG_ASSERT(animFactor.IsPaused());
        SG_ASSERT(3 == a.HitCount());
        SG_ASSERT(1 == b.HitCount());
        SG_ASSERT(0 == c.HitCount());
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.375f == t);
        SG_ASSERT(3 == a.HitCount());
        SG_ASSERT(1 == b.HitCount());
        SG_ASSERT(0 == c.HitCount());

        animFactor.Play();
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.375f == t);
        SG_ASSERT(animFactor.IsPaused());
        SG_ASSERT(3 == a.HitCount());
        SG_ASSERT(1 == c.HitCount());
        SG_ASSERT(0 == d.HitCount());

        animFactor.Play();
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.375f == t);
        SG_ASSERT(animFactor.IsPaused());
        SG_ASSERT(3 == a.HitCount());
        SG_ASSERT(1 == b.HitCount());
        SG_ASSERT(1 == c.HitCount());
        SG_ASSERT(1 == d.HitCount());

        animFactor.Play();
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.625f == t);
        SG_ASSERT(4 == a.HitCount());
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.875f == t);
        SG_ASSERT(5 == a.HitCount());
        SG_ASSERT(1 == f.HitCount());

        animFactor.SetValue(0.625f);
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.875f == t);
        SG_ASSERT(6 == a.HitCount());
        SG_ASSERT(1 == f.HitCount());

        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(1.f == t);
        SG_ASSERT(8 == a.HitCount());

        animFactor.Backward();
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.75f == t);
        SG_ASSERT(10 == a.HitCount());
        SG_ASSERT(0 == e.HitCount());
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.625f == t);
        SG_ASSERT(animFactor.IsPaused());
        SG_ASSERT(10 == a.HitCount());
        SG_ASSERT(1 == e.HitCount());
        animFactor.Play();
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.375f == t);
        SG_ASSERT(13 == a.HitCount());
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.125f == t);
        SG_ASSERT(14 == a.HitCount());
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        SG_ASSERT(15 == a.HitCount());

        animFactor.Forward();
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        animFactor.Pause();
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        SG_ASSERT(17 == a.HitCount());
    }

    {
        timeServer.SetTick(0);
        EventListenerForTest a;
        CyclicAnimFactorWithEventsOnTimeServer<TimeServerForTest> animFactor(8.f);
        animFactor.AddEvent(0.00f, &a, AnimFactorEventFlag::Forward_Backward);
        animFactor.AddEvent(0.20f, &a, AnimFactorEventFlag::Forward);
        animFactor.AddEvent(0.40f, &a, AnimFactorEventFlag::Backward);
        animFactor.AddEvent(0.60f, &a, AnimFactorEventFlag::Forward);
        animFactor.AddEvent(0.80f, &a, AnimFactorEventFlag::Backward);

        float t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        SG_ASSERT(0 == a.HitCount());
        animFactor.Play();
        t = animFactor.GetValue();
        SG_ASSERT(0.f == t);
        SG_ASSERT(1 == a.HitCount());
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        SG_ASSERT(2 == a.HitCount());
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        SG_ASSERT(2 == a.HitCount());
        timeServer.SetTick(timeServer.Tick() + (6 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        SG_ASSERT(5 == a.HitCount());
        timeServer.SetTick(timeServer.Tick() + (10 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        SG_ASSERT(8 == a.HitCount());
        animFactor.Reverse();
        timeServer.SetTick(timeServer.Tick() + (2 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.25f == t);
        SG_ASSERT(9 == a.HitCount());
        timeServer.SetTick(timeServer.Tick() + (6 << 8));
        t = animFactor.GetValue();
        SG_ASSERT(0.5f == t);
        SG_ASSERT(11 == a.HitCount());
    }
}
//=============================================================================
}
}
#endif
