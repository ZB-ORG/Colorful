/*******************************************************************************
 *                              US212A
 *                            Module: music engine
 *                 Copyright(c) 2003-2012 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>    <time>           <version >             <desc>
 *      fiona.yang  2014-11-05        1.0              create this file
 *******************************************************************************/

#include "app_uengine.h"

#include "audio_device.h"

//应用自己需要的软定时器个数
#define APP_TIMER_COUNT		3

//for share status inquiry
uengine_status_t g_cur_play_status[2];

uengine_status_t* g_cur_play_status_p;

//for stop usb
uint32 g_stop_count;

//globle variable
//保存中间件句柄
void *g_mmm_pp_handle = NULL;

//中间件状态检测定时器
int32 g_status_timer;

//hid事务处理定时器
int32 g_hid_timer;

usound_status_t g_status_data;

mmm_pp_ainout_setting_t g_ain_out_setting;

uint32 g_volume_syn_cfg;

//中间件状态检测标志位
uint8 g_check_status_flag;

//usound engine延时参数
uint8 g_delay_time;

uint8 g_auto_play_flag;

//控件返回值
app_result_e g_uengine_result;

//vram恢复变量
//mengine_config_t g_bkinfo_bak;

//eq data
//mmm_mp_eq_info_t g_eq_info;

//app_timer_vector：应用软定时器数组指针，指向应用空间的软定时器数组
app_timer_t g_uengine_timer_vector[APP_TIMER_COUNT];

OS_STK *ptos = (OS_STK *) AP_UENGINE_STK_POS;

INT8U prio = AP_UENGINE_PRIO;

//因TTS播报而暂停播放音乐，TTS播报结束后需要恢复播放音乐
//bool stop_by_tts_flag;
//driver select  USOUND_TYPE
uint32 g_usound_cfg;

//for tts flags;
uint32 g_stop_by_tts;
uint32 energy_count;

uengine_sync_volume_t g_sync_vol_share;

uengine_sync_volume_t* g_share_ptr;

//for dae adjust freq
dae_config_t *g_dae_cfg_shm;

uengine_config_t g_usoud_config_val;

bool g_uspeaker_24bit;
/******************************************************************************/
/*!
 * \par  Description:
 * \void _load_cfg(void)
 * \读取配置信息
 * \param[in]    void  para1
 * \param[out]   none
 * \return       void the result
 * \retval
 * \retval
 * \ingroup      uengine_main.c
 * \note
 */
/*******************************************************************************/
void _load_cfg(void)
{
    g_usound_cfg = com_get_config_default(USOUND_TYPE);

    sys_vm_read(&g_usoud_config_val, VM_AP_USOUND, sizeof(uengine_config_t));

    if (g_usoud_config_val.magic != VRAM_MAGIC(VM_AP_USOUND))
    {
        g_usoud_config_val.magic = VRAM_MAGIC(VM_AP_USOUND);

        g_usoud_config_val.dac_sample = 44;

        g_usoud_config_val.adc_sample = 44;
    }
}

/******************************************************************************/
/*!
 * \par  Description:
 * \void _save_cfg(void)
 * \保存配置信息
 * \param[in]    void  para1
 * \param[out]   none
 * \return       void the result
 * \retval
 * \retval
 * \ingroup      uengine_main.c
 * \note
 */
/*******************************************************************************/
void _save_cfg(void)
{
    g_usoud_config_val.dac_sample = g_ain_out_setting.dac_sample;

    g_usoud_config_val.adc_sample = g_ain_out_setting.adc_sample;

    sys_vm_write(&g_usoud_config_val, VM_AP_USOUND, sizeof(uengine_config_t));
}

/******************************************************************************/
/*!
 * \par  Description:
 * 配置音频输入参数
 * \param[in]    none
 * \param[out]   none
 * \return       none
 * \ingroup      uengine_main.c
 * \note
 *******************************************************************************/
void _set_ainout_param(mmm_pp_ainout_setting_t* param_ptr)
{

    if (g_usound_cfg == 0)
    {
        param_ptr->input_type = MMM_PP_USOUND_IN;//usb up and down
    }
    else
    {
        param_ptr->input_type = MMM_PP_USPEAKER_IN;//usb down only
    }

    param_ptr->asrc_index = K_OUT1_P2_IN_U1;//asrc index

    param_ptr->dac_sample = g_usoud_config_val.dac_sample;//48;//48k

    param_ptr->dac_chanel = 1;//dac 1

    param_ptr->asrc_mode_sel = 2;//param table index

    param_ptr->adc_sample = g_usoud_config_val.adc_sample;//48;//adc sample rate

    param_ptr->adc_insel = AI_SOURCE_ADFMIC;

    param_ptr->ain_gain = 7;

    param_ptr->adc_gain = 0;

}

/******************************************************************************/
/*!
 * \par  Description:
 * 获取USB相关的配置信息
 * \param[in]    none
 * \param[out]   none
 * \return       none
 * \ingroup      uengine_main.c
 * \note
 *******************************************************************************/
void _set_usb_info(void)
{
    uint8 info_str[16];

    //vid
    com_get_config_struct(USB_VID, info_str, 6);

    ud_set_config(0, info_str, 6);

    //pid
    com_get_config_struct(USB_SOUND_PID, info_str, 6);

    ud_set_config(1, info_str, 6);

    //vendor
    com_get_config_struct(INF_USB_VENDOR, info_str, 8);

    ud_set_config(2, info_str, 8);

    //product name
    com_get_config_struct(INF_SOUND_PRODUCT_NAME, info_str, 16);

    ud_set_config(3, info_str, 16);

    //set volume
    com_get_config_struct(INF_VOLUME_FLAG, info_str, 1);

    g_volume_syn_cfg = info_str[0];

    ud_set_config(4, info_str, 1);

}

/******************************************************************************/
/*!
 * \par  Description:
 * \int _app_init(void)
 * \初始化引擎、读取配置信息、初始化文件选择器、中间件
 * \param[in]    void  para1
 * \param[out]   none
 * \return       int the result
 * \retval           1 sucess
 * \retval          -1 failed
 * \ingroup      uengine_main.c
 * \note
 */
/*******************************************************************************/
int _app_init(void)
{
    int app_init_ret = 0;
    //bool init_fsel_ret;
    char *mmm_name =
    { "mmm_pp.al" };
    scan_isr_cbk tmp_scan_isr_cbk;
    
    testval_t us_state = {0};
    
    sys_vm_read(&us_state,VM_API_FLAG,sizeof(testval_t));
    
    //读取配置信息 (包括vm信息)
    _load_cfg();

    //初始化applib库，后台AP
    applib_init(APP_ID_UENGINE, APP_TYPE_CONSOLE);

    //初始化 applib 消息模块
    applib_message_init();

    //初始化软定时器
    init_app_timers(g_uengine_timer_vector, APP_TIMER_COUNT);

    if (g_usound_cfg == 0)
    {
        //usound drv
        sys_drv_install(DRV_GROUP_UD, 0, "usound.drv");

    }
    else if (g_usound_cfg == 1)
    {
        //speaker drv
        sys_drv_install(DRV_GROUP_UD, 0, "uspeaker.drv");
    }
    else if (g_usound_cfg == 2)
    {
        g_uspeaker_24bit = 1;
        sys_drv_install(DRV_GROUP_UD, 0, "usound24.drv");
    }
    else
    {
        ;//for QAC
    }

    if(0 == g_uspeaker_24bit)
    {
        //set param init
        _set_ainout_param(&g_ain_out_setting);

        //加载中间件驱动
        sys_load_mmm(mmm_name, TRUE);//待完善
            
        //must after usb driver install
        app_init_ret = uegine_player_open();
    }
    else
    {
        ;//nothing for 24bit
    }

    _set_usb_info();

    //disable detect when init
    DISABLE_USB_DETECT();

    ud_module_start(&tmp_scan_isr_cbk);
    key_register_scan_isr_cbk(tmp_scan_isr_cbk);

    //enable detect after init
    ENABLE_USB_DETECT();
    
    if((us_state.api_flag & 0x08) != 0)
    {
        g_auto_play_flag = 1;
        us_state.api_flag &= ~0x08;
        sys_vm_write(&us_state,VM_API_FLAG,sizeof(testval_t));
    }
    
    //第一次进入立即查询状态，以更新mengine内容
    g_check_status_flag = TRUE;

    return app_init_ret;
}

/******************************************************************************/
/*!
 * \par  Description:
 * \bool _app_deinit(void)
 * \退出app的功能函数,保存配置信息
 * \param[in]    void  para1
 * \param[out]   none
 * \return       bool the result
 * \retval           1 sucess
 * \retval           0 failed
 * \ingroup      uengine_main.c
 * \note
 */
/*******************************************************************************/
bool _app_deinit(void)
{
    testval_t us_state = {0};
    
    sys_vm_read(&us_state,VM_API_FLAG,sizeof(testval_t));
    
    if(0 == g_uspeaker_24bit)
    {
        //must before usb driver unstall
        uegine_player_close();
    }
    else
    {
        ud_set_cmd(SET_USPEAKER_24BIT,USPEAKER24_PAUSE);
    }

    //if is play then stop
    if (g_status_data.start_play == 1)
    {
        uint8 i = 0;
        ud_set_cmd(SET_HID_OPERS, 0x08);
        while (i < 5)
        {
            ud_hid_deal();
            i++;
            sys_os_time_dly(1);
        }
        us_state.api_flag |= 0x08;
        sys_vm_write(&us_state,VM_API_FLAG,sizeof(testval_t));
    }

    //disable detect when init
    DISABLE_USB_DETECT();
    //必须先停止usb 再停止dsp
    key_register_scan_isr_cbk(NULL);
    ud_module_stop();

    //enable detect after init
    ENABLE_USB_DETECT();

    if (0 == g_uspeaker_24bit)
    {
    //卸载解码驱动
        sys_free_mmm(TRUE);
    }
    else
    {
        ccd_i2s_pa_set_clock(48);
        //sys_unlock_adjust_freq();
    }
    
    sys_drv_uninstall(DRV_GROUP_UD);

    //执行applib库的注销操作
    applib_quit();

    //save config 写vram
    _save_cfg();

    return TRUE;
}

/******************************************************************************/
/*!
 * \par  Description:
 * \int main(int argc, const char *argv[])
 * \app入口函数
 * \param[in]    argc  para1
 * \param[in]    argc  para2
 * \param[out]   none
 * \return       int the result
 * \retval           1 sucess
 * \retval           0 failed
 * \ingroup      mengine_main.c
 * \note
 */
/*******************************************************************************/
int main(int argc, const char *argv[])
{

    app_result_e result;

    g_cur_play_status_p = &g_cur_play_status[0];//应用加载时初始化为0，不用再初始化了

    //创建共享查询，这时必须保证前台来获取状态时是可用的
    g_cur_play_status_p = sys_share_query_creat(APP_ID_UENGINE, g_cur_play_status_p, sizeof(uengine_status_t));

    if (g_cur_play_status_p == NULL)
    {
        PRINT_ERR("usound share query create fail!\n");
        while (1)
        {
            ;//do noting
        }

    }

    //创建共享内存
    //libc_memcpy(g_share_ptr, &(setting_comval->dae_cfg), sizeof(dae_config_t));
    g_share_ptr = &g_sync_vol_share;

    if (sys_shm_creat(SHARE_MEM_ID_USOUND, g_share_ptr, sizeof(uengine_sync_volume_t)) == -1)
    {
        PRINT_ERR("usound shm create fail!!");
        while (1)
        {
            ; //nothing for QAC
        }
    }

    //get for dae freqency
    //更新DAE配置参数到共享内存
    g_dae_cfg_shm = (dae_config_t *) sys_shm_mount(SHARE_MEM_ID_DAECFG);
    if (g_dae_cfg_shm == NULL)
    {
        PRINT_ERR("dae_cfg shm not exist!!");
        while (1)
        {
            ; //nothing for QAC
        }
    }

    //初始化
    _app_init();

    //循环
    result = uengine_control_block();//app功能处理

    //退出
    _app_deinit();

    //销毁共享内存
    sys_shm_destroy(SHARE_MEM_ID_USOUND);

    //销毁共享查询
    if (sys_share_query_destroy(APP_ID_UENGINE) == -1)
    {
        PRINT_ERR("usound share query destroy fail!\n");
        while (1)
        {
            ;//do noting
        }
    }

    return result;
}
