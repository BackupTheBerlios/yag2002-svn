/***************************************************************************
 AUTHOR: Thomas Tong - 10/5/99 3:53PM
 Timer
 **************************************************************************/
#ifndef _SYSTIMER_H_
#define _SYSTIMER_H_

#include "Utility\utlSingleton.h"

class sysTimer
{
public:
    DECL_SINGLETON(sysTimer);

    /***************************************************************************
    Mutators
    Pulse           -- Must be called each update in order for timing information to update
    GetTime         -- Current MSecs to current update
    GetTimeDiff     -- Num MSecs between this and last update
    **************************************************************************/
    sysTimer();
    
    void Reset                  (void);
    void Pulse                  (void);
    unsigned int  GetTime       (void) {return m_UT_TimeCurrent;};
    unsigned int  GetTimeDiff   (void) {return m_UT_TimeDiff;};

    float         GetTimeF      (void) {return m_UT_TimeCurrentF;};
    float         GetTimeDiffF  (void) {return m_UT_TimeDiffF;};

    /***************************************************************************
    GetTimeCurrent  
    -- NOTE! Internal or Profiling use only! Use GetTime() if 
    -- On PS2 Wraps around every 14 seconds, 
    -- On PC, Will not reflect game time.
    **************************************************************************/    
    static unsigned int  GetTimeCurrent (void);
    
    
    /***************************************************************************
    Use for lengthly timings (seconds accuracy) WARNING :VERY VERY SLOW on PS2
    ***************************************************************************/
    static unsigned int  GetSystemTime  (void);

private:
    
    unsigned int m_UT_TimeHardware;

    unsigned int m_UT_TimeCurrent;
    unsigned int m_UT_TimeDiff;

    float m_UT_TimeCurrentF;
    float m_UT_TimeDiffF;
};


#endif
