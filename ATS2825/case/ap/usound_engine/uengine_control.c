/*******************************************************************************
 *                              US211A
 *                            Module: music engine
 *                 Copyright(c) 2003-2011 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>        <time>           <version >            <desc>
 *     fiona.yang     2011-09-06 15:00     1.0             build this file
 *******************************************************************************/
#include "app_uengine.h"

/******************************************************************************/
/*!
 * \par  Description:
 * \int32 uegine_player_open(void)
 * \打开中间件
 * \param[in]    void
 * \param[out]   none
 * \return       none
 * \retval       none
 * \ingroup      uengine_control.c
 * \note
 */
/*******************************************************************************/
int32 uegine_player_open(void)
{
    int open_ret = 0;
    if(0 == g_uspeaker_24bit)
    {
        //bool init_fsel_ret;
        //char *mmm_name =
        //{ "mmm_pp.al" };

        //加载中间件驱动
        //sys_load_mmm(mmm_name, TRUE);//待完善

        if (g_mmm_pp_handle != NULL)
        {
            return 0;
        }

        adjust_freq_set_level(AP_BACK_HIGH_PRIO, FREQ_LEVEL10, FREQ_LEVEL10);

        adjust_freq_add_value(0, g_dae_cfg_shm->run_freq);

        //初始化中间件库
        open_ret = mmm_pp_cmd(&g_mmm_pp_handle, MMM_PP_OPEN, (unsigned int) NULL);

        if (open_ret != -1)
        {
            open_ret = mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_AINOUT_OPEN, (unsigned int) &g_ain_out_setting);
        }

		//输出声道处理选择
		//#define OUTPUT_CHANNEL_NORMAL      0 //正常，中间件会初始化为正常
		//#define OUTPUT_CHANNEL_L_R_SWITCH  1 //左右声道互换 L = R0, R = L0
		//#define OUTPUT_CHANNEL_L_R_MERGE   2 //左右声道合并后输出到左声道和右声道 L = R = (L0 + R0) / 2
		//#define OUTPUT_CHANNEL_L_R_ALL_L   3 //左右声道都输出左声道 L = R = L0
		//#define OUTPUT_CHANNEL_L_R_ALL_R   4 //左右声道都输出右声道 L = R = R0
		//mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_SET_OUTPUT_CHANNEL, (unsigned int) OUTPUT_CHANNEL_NORMAL);

        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_PLAY, (unsigned int) NULL);

        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_GET_ENERGY_INFO, (unsigned int) &(g_app_info_state_all.p_energy_value));

        g_app_info_state_all.energy_available_flag = TRUE;

        //start get data timer
        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_START_SEND, (unsigned int) NULL);

    	ud_set_cmd(SET_ADJUST_TIMER, 1);
    }
    else
    {
        ud_set_cmd(SET_USPEAKER_24BIT,USPEAKER24_SAMPLE_RATE);
        ud_set_cmd(SET_USPEAKER_24BIT,USPEAKER24_PLAY);
    }
    return open_ret;
}

/******************************************************************************/
/*!
 * \par  Description:
 * \void uegine_player_close(void)
 * \关闭中间件
 * \param[in]    void
 * \param[out]   none
 * \return       none
 * \retval       none
 * \ingroup      uengine_control.c
 * \note
 */
/*******************************************************************************/

void uegine_player_close(void)
{
    if(0 == g_uspeaker_24bit)
    {
        uint32 fade_out_use = 0;

        ud_set_cmd(SET_ADJUST_TIMER, 0);

        if (g_mmm_pp_handle == NULL)
        {
            return;
        }

        g_app_info_state_all.energy_available_flag = FALSE;

        if (g_status_data.start_play == 0)
        {
            mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_FADE_OUT_TIME, (unsigned int) fade_out_use);
        }

        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_STOP, (unsigned int) NULL);

        //stop get data timer
        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_STOP_SEND, (unsigned int) NULL);

        //关闭声音设备
        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_AINOUT_CLOSE, (unsigned int) NULL);

        //关闭中间件设备
        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_CLOSE, (unsigned int) NULL);

        g_mmm_pp_handle = NULL;

        adjust_freq_add_value(0, 0);

        adjust_freq_set_level(AP_BACK_HIGH_PRIO, FREQ_NULL, FREQ_NULL);

        //卸载解码驱动
        //sys_free_mmm(TRUE);
    }
    else
    { 
        ud_set_cmd(SET_USPEAKER_24BIT,USPEAKER24_PAUSE);
    }
}

/******************************************************************************/
/*!
 * \par  Description:
 * \void uengine_status_deal(void)
 * \引擎状态处理,处理当前音乐播放状态
 * \param[in]    void  para1
 * \param[out]   none
 * \return       void the result
 * \retval       none
 * \ingroup      uengine_control.c
 * \note
 */
/*******************************************************************************/
void uengine_status_deal(void)
{
    ;//do nothing
}
/******************************************************************************/
/*!
 * \par  Description:
 * \void uegine_check_status(void)
 * \检测引擎状态定时器函数，设置标志位
 * \param[in]    void
 * \param[out]   none
 * \return       none
 * \retval       none
 * \ingroup      uengine_control.c
 * \note
 */
/*******************************************************************************/
void uegine_check_status(void)
{
    g_check_status_flag = TRUE;
}
//这里增加能量检测是为了解决win7下大部分播放器在暂停时还会发送20多m左右的ISO数据过来
//导致小机没办法正常播放当前的播放状态，从而出现播放和播报混乱的问题
void check_energy_info(void)
{
    short *p_energy_mean;
    short *p_energy_max;

    if (g_app_info_state_all.energy_available_flag == TRUE)
    {
        p_energy_mean = g_app_info_state_all.p_energy_value;
        p_energy_max = g_app_info_state_all.p_energy_value + 1;
        if ((*p_energy_mean == 0) && (*p_energy_max < 5))
        {
            energy_count++;
            if (energy_count == MAX_ENERGY_NUM)
            {
                //设置能量低标志，告知驱动当前处理暂停状态
                ud_set_cmd(SET_PLAY_FLAG, 3);
            }
        }
        else
        {
            if (energy_count >= MAX_ENERGY_NUM)
            {
                //清除能量低标志，恢复正常状态处理
                ud_set_cmd(SET_PLAY_FLAG, 4);
            }
            energy_count=0;
        }
        //libc_print("\n energy_mean:", *p_energy_mean,2);
        //libc_print("\n energy_max:", *p_energy_max,2);
    }
}

/******************************************************************************/
/*!
 * \par  Description:
 * \void uengine_control_block(void)
 * \引擎功能处理函数
 * \param[in]    void  para1
 * \param[out]   none
 * \return       void the result
 * \retval       none
 * \ingroup      uengine_control.c
 * \note
 */
/*******************************************************************************/
app_result_e uengine_control_block(void)
{
    //mmm_pp_status_t play_status;

    private_msg_t cur_msg_val;

    uint32 hid_count = 0;

    app_result_e uengine_retval = RESULT_NULL;

    uint8 i = 0;

    //250ms处理hid 事务
    //g_hid_timer = set_app_timer(APP_TIMER_ATTRB_CONTROL, 500, uegine_deal_hid);
    uint32 auto_play_count = 0;

    uint8 auto_play_flag = 0;
    
    while (1)
    {
        //查询当前播放状态
        ud_get_status(&g_status_data);
        if (g_status_data.start_play == 0)
        {
            g_stop_count++;

            if (g_stop_count >= 5)
            {
                g_cur_play_status_p->play_status = 0;
            }
        }
        else
        {
            g_stop_count = 0;

            g_cur_play_status_p->play_status = 1;
        }

        g_cur_play_status_p->line_status = g_status_data.line_sta;

        //更新共享内存
        g_cur_play_status_p = sys_share_query_update(APP_ID_UENGINE);

        if (((g_volume_syn_cfg & 0x02) != 0) && (g_status_data.volume_chg != 0))
        {

            ud_set_cmd(SET_VOLUME_FLAG, 0);

            //change volume
            uengine_sync_volume(g_status_data.volume_value);
           
        }

        //hid change

        hid_count++;

        if ((g_status_data.hid_idle_time != 0) && (g_status_data.hid_idle_time < (hid_count * 10)))
        {
            hid_count = 0;

            ud_set_cmd(SET_HID_CHANGE, 0);
        }

        //play flag need set sample rate
        if ((g_status_data.start_play == 0x01) && (g_status_data.set_sample == 1))
        {
            if (g_mmm_pp_handle != NULL)
            {
	            //set adjust
           	 	ud_set_cmd(SET_ADJUST_TIMER, 0);

                mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_SET_PARAM, (unsigned int) &g_status_data.sample_rate);

                g_ain_out_setting.dac_sample = (int32) g_status_data.sample_rate;

                g_ain_out_setting.adc_sample = (int32) g_status_data.sample_rate;

                //clr flag
                ud_set_cmd(SET_SAMPLE_RATE, 0);

            	ud_set_cmd(SET_ADJUST_TIMER, 1);
            }
            else if(1 == g_uspeaker_24bit)
            {
                if(g_ain_out_setting.dac_sample != (int32) g_status_data.sample_rate)
                {
                    ccd_i2s_pa_set_clock(g_status_data.sample_rate);
                }
                
                g_ain_out_setting.dac_sample = (int32) g_status_data.sample_rate;
                
                g_ain_out_setting.adc_sample = (int32) g_status_data.sample_rate;

                //ccd_i2s_pa_set_clock(0xff);
                
                ud_set_cmd(SET_USPEAKER_24BIT,USPEAKER24_SAMPLE_RATE);
            }
            else
            {
                ;//for QAC
            }
        }

        if ((g_status_data.start_record == 0x01) && (g_status_data.set_sample == 1))
        {
            //mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_SET_PARAM, (unsigned int) &g_status_data.sample_rate);
            if (g_mmm_pp_handle != NULL)
            {
                mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_SET_PARAM, (unsigned int) &g_status_data.sample_rate);

                g_ain_out_setting.dac_sample = (int32) g_status_data.sample_rate;

                g_ain_out_setting.adc_sample = (int32) g_status_data.sample_rate;

                //clr flag
                ud_set_cmd(SET_SAMPLE_RATE, 0);
            }
        }

        //获取并处理详细
        if (get_app_msg(&cur_msg_val) == TRUE)
        {
            uengine_retval = uengine_message_deal(&cur_msg_val);
        }

        if (uengine_retval == RESULT_APP_QUIT)
        {
            break;
        }
        check_energy_info();
        if (i == 10)
        {
            ud_hid_deal();
            i = 0;
        }
        if (auto_play_count >= 100)
        {
            auto_play_flag = 1;
            auto_play_count = 0;
            if((g_status_data.start_play == 0)&&(g_auto_play_flag == 1))
            {
                ud_set_cmd(SET_HID_OPERS, 0x08);
                g_auto_play_flag = 0;
            }
            else
            {
                g_auto_play_flag = 0;
            }
        }
        //挂起10ms 多任务交互
        sys_os_time_dly(1);
        
        if(auto_play_flag == 0)
        {
            auto_play_count++;
        }
        
        i++;

    }

    //kill_app_timer(g_hid_timer);

    return uengine_retval;
}
/******************************************************************************/
/*!
 * \par  Description:
 * \app_result_e uengine_reply_msg(void* msg_ptr, bool ret_vals)
 * \回复同步消息
 * \param[in]    void  msg_ptr 消息结构体
 * \param[in]    bool  ret_vals 相关事件处理结果 TRUE 成功 FALSE 失败
 * \param[out]   none
 * \return       app_result_E
 * \retval       RESULT_IGNOR 忽略
 * \ingroup      uengine_control.c
 * \note  回复消息时，根据ret_vals的结果，设置应答成功或失败
 */
/*******************************************************************************/
void uengine_reply_msg(void* msg_ptr, bool ret_vals)
{
    //消息指针
    private_msg_t* data_ptr = (private_msg_t*) msg_ptr;

    if (ret_vals == TRUE)
    {
        data_ptr->reply->type = MSG_REPLY_SUCCESS;
    }
    else
    {
        data_ptr->reply->type = MSG_REPLY_FAILED;
    }

    //回复同步消息(发送信号量)
    libc_sem_post(data_ptr->sem);

    //return;
}

