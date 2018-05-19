#include "app_linein_eg.h"

//数字通道时的播放
bool play(void)
{
    int result;

    bool bret = TRUE;
       
    //与TTS复用线程，必须重新设置频率
    adjust_freq_set_level(AP_BACK_HIGH_PRIO, FREQ_LEVEL5, FREQ_LEVEL2);

    adjust_freq_add_value(0, g_dae_cfg_shm->run_freq);
    
    //停止 or 暂停时 发送播放命令
    if (g_eg_status_p->play_status == linein_play_status_stop)
    {
        //初始化中间件库
        mmm_pp_cmd(&g_mmm_pp_handle, MMM_PP_OPEN, (unsigned int) NULL);

        //set sound open
        com_set_sound_out(SOUND_OUT_RESUME);

        g_eg_status_p->play_status = linein_play_status_play; //设置播放状态

        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_AINOUT_OPEN, (unsigned int) &g_ainout_param);

	    //输出声道处理选择
	    //#define OUTPUT_CHANNEL_NORMAL      0 //正常，中间件会初始化为正常
	    //#define OUTPUT_CHANNEL_L_R_SWITCH  1 //左右声道互换 L = R0, R = L0
	    //#define OUTPUT_CHANNEL_L_R_MERGE   2 //左右声道合并后输出到左声道和右声道 L = R = (L0 + R0) / 2
	    //#define OUTPUT_CHANNEL_L_R_ALL_L   3 //左右声道都输出左声道 L = R = L0
        //#define OUTPUT_CHANNEL_L_R_ALL_R   4 //左右声道都输出右声道 L = R = R0
	    //mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_SET_OUTPUT_CHANNEL, (unsigned int) OUTPUT_CHANNEL_NORMAL);
#ifdef WAVES_ASET_TOOLS
         //发送播放命令     
        result = mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_PLAY, g_support_waves_pc_tools);
#else
        //发送播放命令
        result = mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_PLAY, (unsigned int) NULL);     
#endif

        if (result != 0)
        {
            bret = FALSE;
            _error_handle();
        }
        else
        {
            mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_GET_ENERGY_INFO, (unsigned int) &(g_app_info_state_all.p_energy_value));
            g_app_info_state_all.energy_available_flag = TRUE;

            bret = TRUE;
        }
    }

    return bret;
}

//数字通道时的停止
bool stop(void)
{
    bool bret = TRUE;
    int result = 0;
    
    //mmm_pp_status_t play_status;

    g_app_info_state_all.energy_available_flag = FALSE;
    
    /*mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_GET_STATUS, (unsigned int) &play_status); //获取后暂时没用

    if(play_status.status == MMM_PP_ENGINE_STOPPED)
    {
    	 //return bret;
    	 g_eg_status_p->play_status = linein_play_status_stop;
    }*/

    //正常播放才会停止
    if (g_eg_status_p->play_status == linein_play_status_play)
    {
#ifdef WAVES_ASET_TOOLS
        //停止播放
        result = mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_STOP, g_support_waves_pc_tools);     
#else
        //停止播放
        result = mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_STOP, (unsigned int) NULL);
#endif
        if (result != 0)
        {
            bret = FALSE;
        }
        else
        {           
            bret = TRUE;
        }

        /*if (bret == FALSE)
        {
        }*/ 
        //设置状态
        g_eg_status_p->play_status = linein_play_status_stop;
        //关闭声音设备
        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_AINOUT_CLOSE, (unsigned int) NULL);
        //关闭中间件设备
        mmm_pp_cmd(g_mmm_pp_handle, MMM_PP_CLOSE, (unsigned int) NULL);

        //关闭音频输出
        com_set_sound_out(SOUND_OUT_PAUSE);
    }

    adjust_freq_set_level(AP_BACK_HIGH_PRIO, FREQ_LEVEL2, FREQ_NULL);
    adjust_freq_add_value(0, 0);

    return bret;
}
