/*******************************************************************************
 *                              US282A
 *                 Copyright(c) 2014-2015 Actions (Zhuhai) Microelectronics Co., Limited,
 *                            All Rights Reserved.
 *        brief   主函数，音效调试
 *      <author>       <time>
 *       Wekan        2015-4-13       review
 *******************************************************************************/
#include "common_aset_new_func.h"
#include "common_aset_interface.h"


dae_config_t * __section__(".gl_stub_data") g_p_dae_cfg;

uint32 __section__(".gl_stub_data") g_aset_base_timer;

aset_tools_run_state_t __section__(".gl_stub_data") g_aset_run_state;

bool __section__(".gl_stub_data") g_aset_sync_volume;
bool __section__(".gl_stub_data") g_aset_neednot_take_effect;
uint8 __section__(".gl_stub_data") g_aset_main_gain_value;
uint8 __section__(".gl_stub_data") g_aset_equivalent_gain;
bool __section__(".gl_stub_data") g_aset_main_switch_flag;


#ifdef SUPPORT_ASET_TEST

static void aset_tools_open_ack(uint16 cmd)
{
    int ret_val;
    stub_ext_param_t stub_ext_param;

    uint8 cmd_data[8];

    stub_ext_param.opcode = cmd;
    stub_ext_param.payload_len = 0;
    stub_ext_param.rw_buffer = cmd_data;

    while(1)
    {
        ret_val = stub_ext_write(&stub_ext_param);

        if(ret_val == 0)
        {
            break;
        }
    }
}

uint8 get_audiopp_type(void)
{
    uint32 audiopp_type;
    int ret_val;
    uint8 ret_audiopp_type; 
    static uint8 recv_err_cnt = 0;
        
    retry:
        
    ret_val = aset_read_data(STUB_CMD_AUDIOPP_SELECT, &audiopp_type, 4);

    if (0 == ret_val)
    {
        ret_audiopp_type = (uint8)audiopp_type;
    }
    else
    {
        PRINT_INFO_INT("get audiopp type fail!!",audiopp_type);
            
        recv_err_cnt++;
        if (recv_err_cnt < 10)
        {
            goto retry;
        }
        else
        {
            ret_audiopp_type = SMART_MODE;
            recv_err_cnt = 0;
        }
    }

    return ret_audiopp_type;
}

void aset_reconnect_deal(void)
{
    if((g_aset_run_state.run_state == ASET_TOOLS_NOT_WORK)
            || (g_aset_run_state.run_state == ASET_TOOLS_WORK))
    {
        if((g_app_info_vector[APP_TYPE_GUI].app_id == APP_ID_RADIO)
                || (g_app_info_vector[APP_TYPE_GUI].app_id == APP_ID_LINEIN))
        {
            stub_ioctrl_set(SWITCH_URAM, STUB_USE_URAM0, 0);
        }
        else
        {
            stub_ioctrl_set(SWITCH_URAM, STUB_USE_URAM1, 0);
        }

        stub_ioctrl_set(SET_TIMEOUT, 50, 0);

        if(g_aset_run_state.run_state == ASET_TOOLS_NOT_WORK)
        {
#ifdef WAVES_ASET_TOOLS
            aset_tools_open_ack(STUB_CMD_WAVES_ASET_ACK);
#else 
            aset_tools_open_ack(STUB_CMD_ASET_ACK);
#endif
            g_p_dae_cfg->audiopp_type = get_audiopp_type();
            
            aset_write_case_info();

            g_aset_run_state.run_state = ASET_TOOLS_WORK;
        }
    }
    else
    {
        ;//nothing for QAC
    }
}

bool aset_test_init(void)
{
    //更新DAE配置参数到 sys_comval->dae_cfg
    g_p_dae_cfg = &(sys_comval->dae_cfg);

    g_p_dae_cfg->peq_enable = 1;
    g_p_dae_cfg->spk_compensation_enable = 0;
    g_p_dae_cfg->bypass = 0;

    aset_reconnect_deal();

    return TRUE;
}

void aset_test_exit(void)
{

}

void aset_test_set_dae_init(void)
{
    aset_set_dae_init();
}

uint8 reinstall_stub(void)
{
    uint8 stub_type;
    msg_apps_t msg;
    msg_reply_t msg_reply;
    bool need_uninstall_led;

    if (sys_comval->support_led_display == 1)
    {
        need_uninstall_led = TRUE;
    }
    else
    {
        need_uninstall_led = FALSE;
    }

    msg_reply.content = &stub_type;

    msg.type = MSG_STUB_INSTALL_SYNC;
    msg.content.data[0] = need_uninstall_led;

    send_sync_msg(APP_ID_MANAGER, &msg, &msg_reply, 0);

    g_aset_run_state.run_state = ASET_TOOLS_NOT_WORK;

    aset_reconnect_deal();

    return stub_type;
}

void aset_loop_deal(void)
{
    int ret_val;

    aset_status_t aset_status;
    aset_waves_coeff_property_t    coeff_property;
    //libc_print("aset tools state:", g_aset_run_state.run_state, 2);

#if ((CASE_BOARD_TYPE != CASE_BOARD_ATS2823) && (CASE_BOARD_TYPE != CASE_BOARD_DVB_ATS2823))
    if(g_aset_run_state.run_state == ASET_TOOLS_DISCONNECT)
    {
        if (get_cable_state() == CABLE_STATE_CABLE_IN)
        {
            reinstall_stub();
        }
        else
        {
            return;
        }
    }

    if (get_cable_state() != CABLE_STATE_CABLE_IN)
    {
        //如果USB线断开连接，则卸载stub资源，等待重新建立连接
        g_aset_run_state.run_state = ASET_TOOLS_DISCONNECT;
        stub_close();
        sys_free_mmm(FALSE);
        return;
    }
#endif
#ifdef WAVES_ASET_TOOLS
   
   libc_memset(&coeff_property, 0, sizeof(aset_waves_coeff_property_t));

   aset_read_data(STUB_CMD_WAVES_ASET_READ_COEFF_PROPERTY, &coeff_property, sizeof(aset_waves_coeff_property_t));
       
   if (1 == coeff_property.update_flag)
   {
      coeff_property.update_flag = 0;
       
      waves_set_effect_param(SET_WAVES_COEFF_PROPERTY,&coeff_property);   

      //类型改成void*
      ret_val = aset_read_data(STUB_CMD_WAVES_ASET_WRITE_COEFF_CONTENT, (void*)ASET_READ_DATA_BUF,coeff_property.length);       
        
      if (0 != ret_val)
      {

      }
      else
      { 
         waves_set_effect_param(SET_WAVES_EFFECT_PARAM,(uint8*)ASET_READ_DATA_BUF);  
      }
   }
  
 #else  
    ret_val = aset_read_data(STUB_CMD_ASET_READ_STATUS, &aset_status, sizeof(aset_status));

 #endif

    if (ret_val == 0)
    {
        if(g_aset_run_state.run_state == ASET_TOOLS_WORK)
        {
            aset_cmd_deal(&aset_status);
            g_aset_run_state.err_cnt = 0;
        }
        else if(g_aset_run_state.run_state == ASET_TOOLS_NOT_WORK)
        {
            aset_write_case_info();
            g_aset_run_state.run_state = ASET_TOOLS_WORK;
        }
        else
        {
            ;//nothing for QAC
        }
    }
    else
    {
        if(g_aset_run_state.err_cnt > 4)
        {
            if(g_aset_run_state.run_state == ASET_TOOLS_WORK)
            {
                g_aset_run_state.run_state = ASET_TOOLS_NOT_WORK;
            }
        }
        else
        {
            g_aset_run_state.err_cnt++;
        }
    }
}

void aset_loop_deal(void) __FAR__;

void __section__(".rcode") aset_test_loop_deal(void)
{
    uint32 tmp_ab_timer = sys_get_ab_timer();

#ifdef WAVES_ASET_TOOLS 

    aset_loop_deal();

#else
    //200ms执行一次任务
    if ((tmp_ab_timer - g_aset_base_timer) >= 200)
    {
        g_aset_base_timer = tmp_ab_timer;

        aset_loop_deal();

    }
#endif

}

#endif
