#ifndef PRINT_LOG_MW_H_H
#define PRINT_LOG_MW_H_H

//#include <stdarg.h>

/* Debug level defines. */
#define DBG_LEVEL_NONE   ((unsigned short) 0x0000)
#define DBG_LEVEL_ERROR  ((unsigned short) 0x0001)
#define DBG_LEVEL_API    ((unsigned short) 0x0002)
#define DBG_LEVEL_INFO   ((unsigned short) 0x0004)
#define DBG_LEVEL_SANITY   ((unsigned short) 0x0008)

#define DBG_LEVEL_ALL    ((unsigned short) 0x00ff)
#define DBG_LAYER_APP    ((unsigned short) 0x0100)
//#define DBG_LAYER_MMW    ((unsigned short) 0x0200)
#define DBG_LAYER_MW     ((unsigned short) 0x0400)
#define DBG_LAYER_SYS    ((unsigned short) 0x0800)
#define DBG_LAYER_ALL    ((unsigned short) 0xff00)


#define DBG_ERROR(stat...)  if(DBG_LEVEL_MODULE & DBG_LEVEL_ERROR)dbg_level_printf(stat) \


#define DBG_API(stat...)    if(DBG_LEVEL_MODULE & DBG_LEVEL_API)dbg_level_printf(stat)   \


#define DBG_INFO(stat...)    if(DBG_LEVEL_MODULE & DBG_LEVEL_INFO)dbg_level_printf(stat)  \


#define DBG_SANITY(stat...)  if(DBG_LEVEL_MODULE & DBG_LEVEL_SANITY)dbg_level_printf(stat) \
	

//#define DBG_API_NAME(MODULE_NAME)  DBG_API("<%s>API:%s called!!\n",#MODULE_NAME,__FUNCTION__)



#define DEBUG



extern void  dbg_level_printf(CONST char *fmt, ...  );






#endif









