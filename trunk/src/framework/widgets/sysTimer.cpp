
#include <ctd_frbase.h>

#include "sysTimer.h"
#include "System\sysPlatform.h"
#include "Utility\utlSingleton.h"
#include "Utility\utlError.h"


CREATE_SINGLETON(sysTimer);

/***************************************************************************
 Types and Definations
 **************************************************************************/
static const unsigned int   SEC_FREQ_TO_MSEC_FREQ   = 1000;
static const unsigned int   MAX_DT                  = 100;

//#define USE_GET_TICK_COUNT  // Faster
#define USE_PERFORMANCE_COUNTER  // Accurate


void Init_sysTimer (void);
/***************************************************************************
Timer
**************************************************************************/
sysTimer::sysTimer()
{
    // Do initial pulse
    m_UT_TimeCurrent = 0;
    m_UT_TimeHardware= 0;

    Init_sysTimer();
    Pulse();
}

/***************************************************************************
Mutators
**************************************************************************/
void sysTimer::Reset(void)
{
    Pulse();
    Pulse();
}

void sysTimer::Pulse(void)
{

    // Get time and calculate diff
    unsigned int timeLast;
    timeLast         = m_UT_TimeHardware;
    m_UT_TimeHardware= GetTimeCurrent();
    m_UT_TimeDiff    = m_UT_TimeHardware - timeLast;

    if (m_UT_TimeDiff > MAX_DT)
        m_UT_TimeDiff = MAX_DT;        
    
    m_UT_TimeCurrent += m_UT_TimeDiff;
    m_UT_TimeCurrentF = m_UT_TimeCurrent/ 1000.0f;
    m_UT_TimeDiffF    = m_UT_TimeDiff   / 1000.0f;    
}


/***************************************************************************
Init and timer function. - Plat PC
**************************************************************************/
// Storage of Query Performance Counter variables
#ifdef USE_PERFORMANCE_COUNTER
static __int64      m_UT_QP_Frequency;
static __int64      m_UT_QP_TimeInit;

void Init_sysTimer (void)
{
    //Thows if performance counter could not be found.  Use MM Timer.
    BOOL result;

    // Get the frequency of the performance counter
    result = QueryPerformanceFrequency ((LARGE_INTEGER*) &m_UT_QP_Frequency);
    if (result == FALSE)
        throw -1;

    // Get the startup time
    result = QueryPerformanceCounter    ((LARGE_INTEGER*) &m_UT_QP_TimeInit);
    if (result == FALSE)
        throw -1;

    // Convert freq from seconds to milliseconds
    m_UT_QP_Frequency /= SEC_FREQ_TO_MSEC_FREQ;
}
#endif

#ifdef USE_GET_TICK_COUNT
void Init_sysTimer (void) {}
#endif


void ShutDown_sysTimer(void)
{}


unsigned int sysTimer::GetTimeCurrent(void)
{
#ifdef USE_GET_TICK_COUNT
    return GetTickCount();
#endif

#ifdef USE_PERFORMANCE_COUNTER
    __int64  timeCurrent;
    QueryPerformanceCounter((LARGE_INTEGER*) &timeCurrent);
    timeCurrent -= m_UT_QP_TimeInit;
    timeCurrent /= m_UT_QP_Frequency;
    return  static_cast <unsigned int> (timeCurrent);
#endif
}

unsigned int  sysTimer::GetSystemTime  (void)
{

    return GetTickCount();

}

/***************************************************************************
Static_Init --
**************************************************************************/
