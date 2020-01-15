#ifndef Core_TimeFromWallClock_H
#define Core_TimeFromWallClock_H

#include "Assert.h"
#include "FixedPoint.h"
#include "IntTypes.h"
#include "TimeServer.h"

namespace sg {
//=============================================================================
class TimeFromWallClock : public TimeServer
{
public:
    TimeFromWallClock();
    void Play();
    void Pause();
    void SetMultiplier(float iMultiplier);
    void Update();
    bool IsPaused() const { return m_paused; }
private:
    typedef FixedPoint<i32, 8> ifp8;
private:
    u32 m_prevWallClockTick;
    ifp8 m_multiplier;
    bool m_paused;
};
//=============================================================================
}

#endif
