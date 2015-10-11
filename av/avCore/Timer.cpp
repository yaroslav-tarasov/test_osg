#include "stdafx.h"

#include <utils/performance_counter.h>


#include "avCore.h"


using namespace avCore;

Timer * Timer::g_TimerInstance = NULL;

static const char EVENT_SIMULATION_TIME[] = "simulationTime";
static const char EVENT_RUN_STATUS[] = "runStatus";


//////////////////////////////////////////////////////////////////////////
Timer::Timer()
    : m_SimulationTimeCounter(-1)
    , m_SimulationTimeDelta(0.0)
    , m_SimulationTime(0.0)
    , m_FrameTimeCounter(-1)
    , m_FrameTimeDelta(0.0)
    , m_FrameTime(0.0)
    , m_TimerMode(TM_DEDICATED)
    , m_SimulationTimeScale(1.0)
    , m_FixedSimulationTimeDelta(0.0)
{

}

Timer::~Timer() 
{

}


//////////////////////////////////////////////////////////////////////////
bool Timer::FrameCall()
{
    if (m_TimerMode == TM_LOCAL)
    {
        if (m_FixedSimulationTimeDelta != 0.0)
        {
            m_SimulationTimeDelta = m_FixedSimulationTimeDelta;
            m_SimulationTime += m_SimulationTimeDelta;
        }
        else
        {
            if (m_SimulationTimeCounter == -1) // initialization
                m_SimulationTimeCounter = utils::PerformaceCounter::get_counter();

            const __int64 counter = utils::PerformaceCounter::get_counter();
            const __int64 delta = counter - m_SimulationTimeCounter;
            const double deltaSec  = utils::PerformaceCounter::delta_s(delta);

            m_SimulationTimeCounter = counter;
            m_SimulationTimeDelta = m_SimulationTimeScale * deltaSec;
            m_SimulationTime += m_SimulationTimeDelta;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
void Timer::SetSimulationTimeScale( double scale )
{
    m_SimulationTimeScale = scale;
}

//////////////////////////////////////////////////////////////////////////
void Timer::SetTimerMode( TimerMode timerMode )
{
    m_TimerMode = timerMode;
}

//////////////////////////////////////////////////////////////////////////
Timer::TimerMode Timer::GetTimerMode() const
{
    return m_TimerMode;
}

//////////////////////////////////////////////////////////////////////////
void Timer::SetSimulationTime( double simulationTime )
{
    m_SimulationTime = simulationTime;
}

//////////////////////////////////////////////////////////////////////////
double Timer::GetSimulationTime() const
{
    return m_SimulationTime;
}

//////////////////////////////////////////////////////////////////////////
void Timer::UpdateFrameTime()
{
    if (m_FrameTimeCounter == -1)
        m_FrameTimeCounter = utils::PerformaceCounter::get_counter();
    const __int64 frameTimeCounter = utils::PerformaceCounter::get_counter();
    m_FrameTimeDelta =utils::PerformaceCounter::delta_s(frameTimeCounter - m_FrameTimeCounter);
    m_FrameTimeCounter = frameTimeCounter;
    m_FrameTime += m_FrameTimeDelta;
}

//////////////////////////////////////////////////////////////////////////
double Timer::GetFrameTime() const
{
    return m_FrameTime;
}

//////////////////////////////////////////////////////////////////////////
double Timer::GetFrameTimeDelta() const
{
    return m_FrameTimeDelta;
}

//////////////////////////////////////////////////////////////////////////
void Timer::SetFixedSimulationTimeDelta( double fixedSimulationTimeDelta )
{
    m_FixedSimulationTimeDelta = fixedSimulationTimeDelta;
}

//////////////////////////////////////////////////////////////////////////
Timer * Timer::GetInstance()
{
    avAssert( g_TimerInstance );
    return g_TimerInstance;
}

//////////////////////////////////////////////////////////////////////////
void Timer::Create()
{
    avAssert( g_TimerInstance == NULL );
    g_TimerInstance = new Timer();
}

//////////////////////////////////////////////////////////////////////////
void Timer::Release()
{
    avAssert(g_TimerInstance);
    svReleaseMem(g_TimerInstance);
}

