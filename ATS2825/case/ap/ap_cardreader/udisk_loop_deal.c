/*******************************************************************************
 *                              US212A
 *                            Module: MainMenu
 *                 Copyright(c) 2003-2012 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>    <time>           <version >             <desc>
 *       zhangxs     2011-09-05 10:00     1.0             build this file
 *******************************************************************************/

#include "ap_udisk.h"

const key_event_map_t __section__(".rodata.ke_maplist") udisk_ke_maplist[] =
{
    /*! 按任意按键退出 */
    { { KEY_NEXT_VOLADD, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_PREV_VOLSUB, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_NEXT, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_PREV, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_PLAY, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_PHONEPLAY, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_VADD, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_VSUB, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_ABREC, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_MUTE, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_EQ, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    { { KEY_MODE, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, udisk_key_deal_quit_udisk},
    /*! 忽略关机按键 */
    { { KEY_POWER, 0, KEY_TYPE_ALL, KEY_DEAL_FILTER}, NULL},
    /*! 结束标志 */
    { { KEY_NULL, 0, KEY_TYPE_NULL, KEY_DEAL_NULL}, NULL},
};

const sys_event_map_t __section__(".rodata.se_maplist") udisk_se_maplist[] =
{
    //防止漏检
    { {MSG_SD_OUT, 0}, ud_sdout_msg_deal},
    //防止漏检
    { {MSG_SD_IN, 0}, ud_sdin_msg_deal},
    //USB下不响应闹钟
    { {MSG_RTCALARM, 0}, NULL},
#if (SUPPORT_OUTER_CHARGE == 1)
    { {MSG_POWER_OFF, 0}, ud_poweroff_msg_deal},
#endif
    //防止漏检
    { {MSG_USB_UNSTICK, SYSMSG_STOP_TTS_YES}, ud_unstick_msg_deal},
    /*! 结束标志 */
    { {MSG_NULL, 0}, NULL},
};

/******************************************************************************/
/*! app_result_e udisk_key_deal_quit_udisk(void)
 * \par  Description:
 * \ 按键退出u盘模式
 * \param[in]    none
 * \param[out]   none
 * \return       app_result_e
 * \ingroup      udisk_loop_deal.c
 * \note
 *******************************************************************************/
app_result_e udisk_key_deal_quit_udisk(void)
{
    return RESULT_NEXT_FUNCTION;
}

/******************************************************************************/
/*!
 * \par  Description:void udisk_pop_up_disk_timer(void)
 * \pc弹出u盘处理
 * \param[in]    PARAM_FROM_XXX
 * \param[out]   none
 * \return       the result
 * \retval       0 sucess
 * \retval       1 failed
 * \ingroup      module name
 * \par          exmaple code
 * \code
 *               list example code in here
 * \endcode
 * \note
 *******************************************************************************/
void udisk_pop_up_disk_timer(void)
{
    g_pop_up_time_out = 1;
}

/******************************************************************************/
/*!
 * \par  Description:
 *      MainMenu 应用的入口函数和场景管理器模块
 * \param[in]    PARAM_FROM_XXX
 * \param[out]   none
 * \return       the result
 * \retval       0 sucess
 * \retval       1 failed
 * \ingroup      module name
 * \par          exmaple code
 * \code
 *               list example code in here
 * \endcode
 * \note
 *******************************************************************************/
app_result_e get_message_loop(void)
{
    app_result_e result = RESULT_NULL;

    //counter for no enum check
    uint32 wait_time_before_enum = 0;
    uint8 NTRIRQ_count = 0;
    uint8 NO_NTRIRQ_count = 0;

    ureader_status_t g_udisk_status;
    //
    while (1)
    {

        ud_get_status(&g_udisk_status);

        if (wait_time_before_enum > 400)
        {
            //插入的是充电器,插上usb后长时间没有bus reset
            if ((g_udisk_status.suspend_flag == 0) && (g_udisk_status.reset_count == 0))
            {
                result = RESULT_NEXT_FUNCTION;

                break;
            }
        }
        //读取NTRIRQ判断是否为充电器      
        if(act_readl(Usbirq_hcusbirq)&(0x01<<6))
        {
            act_writel((act_readl(Usbirq_hcusbirq)|(0x01<<6)),Usbirq_hcusbirq);
            NTRIRQ_count ++;
            NO_NTRIRQ_count = 0;
        }
        else
        {
            NO_NTRIRQ_count ++;
        }
        if(NTRIRQ_count >= 5)
        {
            result = RESULT_NEXT_FUNCTION;
            libc_print("charger in",0,0);
            break;
        }
        if(NO_NTRIRQ_count >= 200)
        {
            NTRIRQ_count = 0;
            NO_NTRIRQ_count = 0;
        }

        //1:拔线，2：挂起
        //if ((g_udisk_status.line_sta == 0x01) || (g_udisk_status.line_sta == 0x02))
        if ((g_udisk_status.line_sta == 0x01) || (g_pop_up_time_out == 1))
        {
            result = RESULT_NEXT_FUNCTION;

            break;
        }

        //pc pop up
        if (g_udisk_status.line_sta == 0x02)
        {
            if (g_pop_up_timer == -1)
            {
                g_pop_up_timer = set_app_timer(APP_TIMER_ATTRB_CONTROL | (APP_TIMER_TAG_SYS << 8), 500,
                        udisk_pop_up_disk_timer);
            }
        }

        //switch to adfu
        if (g_udisk_status.switch_to_adfu == 0x01)
        {
            //固件升级
            result = RESULT_ENTER_UPGRADE;
            break;
        }
        else if (g_udisk_status.switch_to_adfu == 0x02)
        {
            libc_print("switch adfu ok", 0, 0);

            //进入stub应用程序
            result = RESULT_ENTER_STUB_DEBUG;

            break;
        }
        else
        {
            ;//do nothing
        }

        if (g_udisk_status.remove_card == 1)
        {
            libc_print("sta out", 0, 0);
            ud_set_cmd(SET_CARD_INOUT, 0);

        }
        //message deal
        result = com_view_loop();

#if (SUPPORT_OUTER_CHARGE == 1)
        if (sys_comval->bat_charge_mode == BAT_CHARGE_MODE_OUTER)
        {
            if (IS_EXTERN_BATTERY_IN() == 1)
            {
                result = RESULT_NEXT_FUNCTION;
                break;
            }
        }
        else
#endif
        {
            if (result > RESULT_COMMON_RESERVE)
            {
                break;
            }
        }

        wait_time_before_enum++;

        //挂起10ms，多任务调度
        sys_os_time_dly(1);
    }
    if (g_pop_up_timer != -1)
    {
        kill_app_timer((int8) g_pop_up_timer);

        g_pop_up_timer = -1;
    }

    return result;

}

