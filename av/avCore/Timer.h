#pragma once

#include <osg/Timer>

namespace avCore
{
class Timer 
{
public:
    enum TimerMode
    {
        TM_DEDICATED, // simulation time from external component 
        TM_LOCAL,     // simulation time synchronized with CPU time 
    };


private:

    static Timer *  g_TimerInstance;

    TimerMode       m_TimerMode;

    __int64         m_SimulationTimeCounter;
    double          m_SimulationTimeDelta;
    double          m_SimulationTime;

    double          m_SimulationTimeScale;
    double          m_FixedSimulationTimeDelta;

    __int64         m_FrameTimeCounter;
    double          m_FrameTimeDelta;
    double          m_FrameTime;


private:
                    Timer();
                    ~Timer();

public:
    virtual bool    FrameCall();

    void            SetTimerMode( TimerMode timerMode );
    TimerMode       GetTimerMode() const;

    void            SetSimulationTime( double simulationTime );
    double          GetSimulationTime() const;

    void            SetSimulationTimeScale( double scale );
    void            SetFixedSimulationTimeDelta( double fixedSimulationTimeDelta );

    void            UpdateFrameTime();
    double          GetFrameTime() const;
    double          GetFrameTimeDelta() const;


    static Timer *  GetInstance();
    static void     Create();
    static void     Release();
};

#define GetTimer() Timer::GetInstance()

}

