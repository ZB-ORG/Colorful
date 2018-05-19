/*******************************************************************************
 *                             HeadFile Include                                *
 *******************************************************************************/
#include "mdw_event.h"
#include "..\Inc\sys_common.h"

#undef  DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE   (DBG_LEVEL_INFO | DBG_LEVEL_ERROR )

/*******************************************************************************
 *                    Global Variables and Function Declare                    *
 *******************************************************************************/
uint8 g_ulMainEventflag = 0;
//uint8 g_ulExtEventflag = 0;
uint32 g_ulBootTicks = 0;


/*******************************************************************************
 *                     External Variables and Function Declare                 *
 *******************************************************************************/


#if 0
void EVENT_FlagSet (uint8 ulEventFlag)
{
    g_ulExtEventflag |= ulEventFlag;
    return;
}


uint8 EVENT_FlagGetClear (void)
{
    uint8 ulTmp = 0;

    ulTmp = g_ulExtEventflag;
    g_ulExtEventflag = 0;

    return ulTmp;
}
#endif

void EVENT_MainFlagSet (uint8 ulEventFlag)
{
    g_ulMainEventflag |= ulEventFlag;
    return;
}


uint8 EVENT_MainFlagGetClear (void)
{
    uint8 ulTmp = 0;

    ulTmp = g_ulMainEventflag;
    g_ulMainEventflag = 0;

    return ulTmp;
}


void SysEvent_Func (void)
{
    g_ulBootTicks++;

    if ( (g_ulBootTicks % 10) == 0)
    {
        EVENT_MainFlagSet (TIMER_10MS_EVENT);
    }

    if ( (g_ulBootTicks % 20) == 0)
    {
        EVENT_MainFlagSet (TIMER_20MS_EVENT);
    }

    if ( (g_ulBootTicks % 100) == 0)
    {
        EVENT_MainFlagSet (TIMER_100MS_EVENT);
    }

    if ( (g_ulBootTicks % 1000) == 0)
    {
        EVENT_MainFlagSet (TIMER_1000MS_EVENT);
    }
}
void SYS_MainTask_Init (void)
{
    GlobalMemoryInit();
}

void SYS_MainTask (void)
{
    uint8 ulMainEvent = 0;
    ulMainEvent = EVENT_MainFlagGetClear();

    if (ulMainEvent & TIMER_10MS_EVENT)
    {
        ;
    }

    /* 20 ms task */
    if (ulMainEvent & TIMER_20MS_EVENT)
    {
        Simulator();
        
        /* Render the simulated result */
        Renderer();
        
        /* Prepare display */
        DisplayManager ( TRUE );

        DmaDoM2Spi();

    }

    /* 100 ms task */
    if (ulMainEvent & TIMER_100MS_EVENT)
    {
        ;

    }

    /* 1000 ms task */
    if (ulMainEvent & TIMER_1000MS_EVENT)
    {
        ;

    }
}


/*******************************************************************************
 *                                End of File                                  *
 *******************************************************************************/

