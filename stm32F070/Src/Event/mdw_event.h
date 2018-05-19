
#ifndef __MDW_EVENT_H__
#define __MDW_EVENT_H__
#include "u_type_def.h"

typedef enum
{
	TIMER_10MS_EVENT		= 1 << 0,
	TIMER_20MS_EVENT		= 1 << 1,
	TIMER_100MS_EVENT		= 1 << 2,
	TIMER_1000MS_EVENT		= 1 << 3,
}eFlagTimer;



extern void SysEvent_Func(void);
extern void SYS_MainTask_Init (void);
extern void SYS_MainTask(void);


#endif
