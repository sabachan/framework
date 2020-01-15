#include "stdafx.h"

#include "TimeFromWallClock.h"

#include "Cast.h"
#include "WindowsH.h"
#include <algorithm>

namespace sg {
//=============================================================================
namespace {
u32 GetWallClockFrequency()
{
    LARGE_INTEGER freq;
    BOOL const rc = QueryPerformanceFrequency(&freq);
    SG_ASSERT_AND_UNUSED(rc);
    u32 const f = checked_numcastable(freq.QuadPart);
    return f;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
u32 GetWallClockTick()
{
    LARGE_INTEGER t;
    BOOL const rc = QueryPerformanceCounter(&t);
    SG_ASSERT_AND_UNUSED(rc);
    u32 tick = u32(t.QuadPart);
    return tick;
}
}
//=============================================================================
TimeFromWallClock::TimeFromWallClock()
    : TimeServer(GetWallClockFrequency())
    , m_multiplier(1)
    , m_prevWallClockTick(0)
    , m_paused(true)
{
    Play();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TimeFromWallClock::Play()
{
    if(m_paused)
    {
        m_prevWallClockTick = GetWallClockTick();
        m_paused = false;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TimeFromWallClock::Pause()
{
    m_paused = true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TimeFromWallClock::SetMultiplier(float iMultiplier)
{
    m_multiplier = ifp8(iMultiplier);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TimeFromWallClock::Update()
{
    if(m_paused)
        return;
    SG_ASSERT(GetWallClockFrequency() == Frequency());
    u32 const wallClockTick = GetWallClockTick();
    i32 const rawDelta = wallClockTick - m_prevWallClockTick;
    SG_ASSERT(rawDelta >= 0);
    SG_ASSERT(Frequency() < 0x7FFFFFFF / 60);
    SG_ASSERT(u32(rawDelta) <= 60 * Frequency()); // Ok if when leaving a breakpoint after 60s.
    i32 const maxDelta = i32(Frequency() >> 3); // No more than 125 ms
    i32 const delta = std::min(rawDelta, maxDelta);
    ifp8 const dtfp8 = delta * m_multiplier;
    i32 const dt = roundi(dtfp8);
    u32 const timeServerTick = Tick();
    SetTick(timeServerTick + dt);
    m_prevWallClockTick = wallClockTick;
}
//=============================================================================
}
