/********************************************************************************
 *        Copyright(c) 2014-2015 Actions (Zhuhai) Technology Co., Limited,
 *                            All Rights Reserved.
 *
 * 描述：蓝牙数传SPP/BLE相关应用层接口，相当于BT STACK的SPP/BLE的适配层。
 * 作者：cailizhen
 ********************************************************************************/

#include "applib.h"
#include "common_func.h"
#include "common_btmanager.h"

int com_btmanager_sppble_send(void *data, uint16 data_len)
{
#if (SUPPORT_RCP_FUNC == 1)
    msg_apps_t rcp_msg;
    rmt_ctrl_pkg_t tmp_rcp;

    tmp_rcp.rcp_data = data;
    tmp_rcp.rcp_len = data_len;
    
    tmp_rcp.rcp_flag = g_rcp_service_type;
    
    tmp_rcp.rcp_remote = g_rcp_remote_number;

    rcp_msg.type = MSG_BTSTACK_RCP_SEND_SYNC;
    rcp_msg.content.addr = &tmp_rcp;
    if (send_sync_msg_btmanager(NULL, &rcp_msg, NULL, 0) == ERR_NO_ERR)
    {
        return 0;
    }
    else
    {
        return -1;
    }
#else
    return 0;
#endif
}

int __section__(".rcode") com_btmanager_sppble_read(uint8 *rcp_buf, uint16 data_len)
{
#if (SUPPORT_RCP_FUNC == 1)
    uint16 read_len, rem_len;

    sys_os_sched_lock();

    if (g_p_rcp_in_data_ctl->length >= data_len)
    {
        if ((g_p_rcp_in_data_ctl->read_ptr + data_len) >= g_p_rcp_in_data_ctl->capacity)
        {
            read_len = g_p_rcp_in_data_ctl->capacity - g_p_rcp_in_data_ctl->read_ptr;
            rem_len = data_len - read_len;
        }
        else
        {
            read_len = data_len;
            rem_len = 0;
        }

        libc_memcpy(rcp_buf, g_p_rcp_in_data_ctl->raw_data + g_p_rcp_in_data_ctl->read_ptr, read_len);

        g_p_rcp_in_data_ctl->read_ptr += read_len;

        if (rem_len != 0)
        {
            libc_memcpy(rcp_buf + read_len, g_p_rcp_in_data_ctl->raw_data, rem_len);

            g_p_rcp_in_data_ctl->read_ptr = rem_len;
        }

        if (g_p_rcp_in_data_ctl->read_ptr >= g_p_rcp_in_data_ctl->capacity)
        {
            g_p_rcp_in_data_ctl->read_ptr = 0;
        }

        g_p_rcp_in_data_ctl->length -= data_len;
    }

    sys_os_sched_unlock();
#endif
    return 0;
}

//启动APK/APP验证
void __section__(".bank") com_btmanater_start_apk_app_verification(void)
{
    if (((g_bt_stack_cur_info.rmt_dev[0].serv_conn & (SPP_CONNECTED | BLE_CONNECTED)) != 0)
            && ((uint8) com_get_config_default(BTMANAGER_VERIFICATION_APK_ENABLE) == 0))
    {
        g_app_info_state.verification_status = VERIFICATION_PASS;
        PRINT_INFO("RCP VERIFICATION NEEDN'T");
    }
    else
    {
        g_app_info_state.verification_status = VERIFICATION_START;
        PRINT_INFO("RCP VERIFICATION START");
    }
    g_sys_counter.apk_app_counter = 0;
}

//断开SPP/BLE连接
void __section__(".bank") com_btmanager_unlink_spp_ble_profile(void)
{
    msg_apps_t msg;

    if ((g_bt_stack_cur_info.rmt_dev[0].serv_conn & SPP_CONNECTED) != 0)
    {
        msg.type = MSG_BTSTACK_SPP_DISCONNECT_SYNC;
    }
    else
    {
        msg.type = MSG_BTSTACK_BLE_DISCONNECT_SYNC;
    }
    msg.content.data[0] = 0;

    send_sync_msg_btmanager(g_bt_stack_cur_info.rmt_dev[0].addr.bytes, &msg, NULL, 0);

    g_app_info_state.verification_status = VERIFICATION_FAIL;

    PRINT_INFO("RCP VERIFICATION OVERTIME, UNLINK");
}

//APK/APP校验
void com_btmanager_apk_app_verification_handle(void)
{
#if (SUPPORT_RCP_FUNC == 1)
    if (((g_btmanager_gl_var.last_spp_ble_status & (SPP_CONNECTED | BLE_CONNECTED)) == 0)
            && ((g_bt_stack_cur_info.rmt_dev[0].serv_conn & (SPP_CONNECTED | BLE_CONNECTED)) != 0)
            && (g_app_info_state.verification_status == VERIFICATION_NULL))
    {
        com_btmanater_start_apk_app_verification();
    }

    if (((g_btmanager_gl_var.last_spp_ble_status & (SPP_CONNECTED | BLE_CONNECTED)) != 0)
            && ((g_bt_stack_cur_info.rmt_dev[0].serv_conn & (SPP_CONNECTED | BLE_CONNECTED)) == 0))
    {
        com_rcp_state_init();
        g_app_info_state.verification_status = VERIFICATION_NULL;
    }

//    if (g_app_info_state.verification_status == VERIFICATION_START)
//    {
//        g_sys_counter.apk_app_counter++;
//        if (g_sys_counter.apk_app_counter >= APK_APP_VERIFICATION_OVERTIME)
//        {
//            //断开SPP/BLE连接
//            com_btmanager_unlink_spp_ble_profile();
//        }
//    }

    g_btmanager_gl_var.last_spp_ble_status = g_bt_stack_cur_info.rmt_dev[0].serv_conn;
#endif
}
