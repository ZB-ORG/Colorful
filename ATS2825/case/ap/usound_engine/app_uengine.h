/********************************************************************************
 *                            Module: music_engine
 *                 Copyright(c) 2003-2011 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>    <time>           <version >             <desc>
 *     fiona.yang   2011-09-05 15:00     1.0             build this file
 ********************************************************************************/
#ifndef __APP_UENGINE_H__
#define __APP_UENGINE_H__

#include "psp_includes.h"
#include "case_include.h"

#include "ud_interface.h"
#include "mmm_pp.h"
#include "usound_common.h"

/*!定义音乐引擎的任务栈和主线程优先级*/
#define    AP_UENGINE_STK_POS  AP_BACK_LOW_STK_POS
#define    AP_UENGINE_PRIO     AP_BACK_LOW_PRIO

#define MAX_ENERGY_NUM     0x08

/*!
 * \brief
 *  引擎信息结构体变量定义
 */
typedef struct
{
    /******************************************************************
     *          引擎状态信息
     *******************************************************************/
    mengine_status_t eg_status;

    /******************************************************************
     *          播放文件的时间和bit率信息
     *******************************************************************/
    mengine_playinfo_t eg_playinfo;

    /******************************************************************
     *          中间件播放文件信息
     *******************************************************************/
    mmm_mp_file_info_t eg_file_info;

    /******************************************************************

     *          设置信息(保存配置)
     *******************************************************************/
    mengine_config_t eg_config;

} uengine_info_t;

typedef struct
{
    /*! 魔数 */
    uint16 magic;

    uint16 reserve;

    uint32 dac_sample;
    uint32 adc_sample;

} uengine_config_t;

//globle variable
//保存中间件句柄
extern void *g_mmm_pp_handle;

//中间件状态检测定时器
extern int32 g_status_timer;

//hid事务处理定时器
extern int32 g_hid_timer;

extern usound_status_t g_status_data;

extern uint32 g_volume_syn_cfg;

//私有消息全局变量
//extern private_msg_t g_cur_msg;
//eq data
extern mmm_mp_eq_info_t g_eq_info;

//中间件状态检测标志位
extern uint8 g_check_status_flag;

//usound engine延时参数
extern uint8 g_delay_time;

//for share memory
extern uengine_status_t* g_cur_play_status_p;

//for stop usb 
extern uint32 g_stop_count;

//for param set 
extern mmm_pp_ainout_setting_t g_ain_out_setting;

extern uint32 g_stop_by_tts;
extern uint32 energy_count;

extern uengine_sync_volume_t* g_share_ptr;

//for dae adjust freq
extern dae_config_t *g_dae_cfg_shm;

extern bool g_uspeaker_24bit;

extern uint8 g_auto_play_flag;

extern app_result_e uengine_control_block(void) __FAR__;

extern int32 uegine_player_open(void) __FAR__;

extern void uegine_player_close(void) __FAR__;

extern app_result_e uengine_message_deal(private_msg_t* cur_msg) __FAR__;

extern app_result_e uengine_set_eq(private_msg_t* cur_msg) __FAR__;

extern app_result_e uengine_play_pause(void) __FAR__;

extern app_result_e uengine_next_song(void) __FAR__;

extern app_result_e uengine_prev_song(void) __FAR__;

extern app_result_e uengine_tts_start(void) __FAR__;

extern app_result_e uengine_tts_stop(void) __FAR__;

extern void uengine_reply_msg(void* msg_ptr, bool ret_vals) __FAR__;

extern app_result_e uengine_sync_volume(uint32 volume) __FAR__;

#endif //__APP_UENGINE_H__
