#ifndef Core_TimeServer_H
#define Core_TimeServer_H

#include "Assert.h"
#include "IntTypes.h"
#include "SmartPtr.h"

namespace sg {
//=============================================================================
// Using a TimeServer is an easy way to share the same time referential between
// multiple systems. The time is expressed as a tick value that can be
// converted to seconds by dividing by frequency. The origin of one TimeSerer
// is not known. Hence, it should only be used to compute durations by
// substracting 2 tick values. Using integers allows for no risk of precision
// lost after a long time. Also, using only u32 should be no problem as only
// time differences should be useful.
class TimeServer : public SafeCountable
{
    SG_NON_COPYABLE(TimeServer)
public:
    TimeServer(u32 iFrequency);
    ~TimeServer();
    u32 Tick() const { SG_ASSERT(0xCAFEFADE == m_guard); return m_tick; }
    u32 Frequency() const { SG_ASSERT(0xCAFEFADE == m_guard); return m_frequency; }
    void SetTick(u32 iTick);
private:
    u32 const m_frequency;
    u32 m_tick;
#if SG_ENABLE_ASSERT
    u32 m_prevTicks[4];
    u32 m_guard;
#endif
};
//=============================================================================
inline TimeServer::TimeServer(u32 iFrequency)
    : m_frequency(iFrequency)
    , m_tick(0)
#if SG_ENABLE_ASSERT
    , m_guard(0xCAFEFADE)
#endif
{
    SG_ASSERT(m_frequency > 0);
#if SG_ENABLE_ASSERT
    for(size_t i=0; i < SG_ARRAYSIZE(m_prevTicks); ++i)
        m_prevTicks[i] = 1;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline TimeServer::~TimeServer()
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0xCAFEFADE == m_guard);
    m_guard = 0;
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline void TimeServer::SetTick(u32 iTick)
{
#if SG_ENABLE_ASSERT
    SG_ASSERT(0xCAFEFADE == m_guard);
    for(size_t i=SG_ARRAYSIZE(m_prevTicks); i > 1; --i)
        m_prevTicks[i-1] = m_prevTicks[i-2];
    m_prevTicks[0] = m_tick;
#endif
    m_tick = iTick;
}
//=============================================================================
}

#endif
