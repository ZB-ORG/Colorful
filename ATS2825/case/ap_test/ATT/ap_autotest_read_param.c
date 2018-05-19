/*******************************************************************************
 *                              US212A
 *                            Module: Picture
 *                 Copyright(c) 2003-2012 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>    <time>           <version >             <desc>
 *       zhangxs     2011-09-19 10:00     1.0             build this file
 *******************************************************************************/
/*!
 * \file     user1_main.c
 * \brief    picture主模块，负责进程初始化，退出处理，场景调度
 * \author   zhangxs
 * \version  1.0
 * \date  2011/9/05
 *******************************************************************************/
#include "ap_manager_test.h"

static uint32 _check_bt_addr_valid(uint8 *bt_addr)
{
    uint32 i;

    for (i = 0; i < 6; i++)
    {
        if ((bt_addr[i] != 0) && (bt_addr[i] != 0xff))
        {
            break;
        }
    }

    if (i == 6)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

//该测试项用于读取蓝牙地址，提供给PC工具进行校验，确认地址是否正确
int act_test_read_bt_addr(void *arg_buffer)
{
    uint32 i;
    uint8 cmd_data[16];
    int ret_val;
    return_result_t *return_data;
    int trans_bytes;
    nvram_param_rw_t param_rw;

    bt_addr_vram_t bt_addr_vram;

    sys_vm_read(&bt_addr_vram, VM_BTSTACK, sizeof(bt_addr_vram));

    if (bt_addr_vram.magic != VRAM_MAGIC(VM_BTSTACK))
    {
        param_rw.logical_index = PARAM_BT_ADDR;
        param_rw.rw_len = 6;
        param_rw.rw_buffer = bt_addr_vram.bt_addr;

        ret_val = base_param_read(&param_rw);

        if (ret_val != 0)
        {
            //没读到蓝牙地址，测试失败
            ret_val = 0;
        }
        else
        {
            if (_check_bt_addr_valid(bt_addr_vram.bt_addr) == TRUE)
            {
                ret_val = 1;
            }
            else
            {
                ret_val = 0;
            }
        }
    }
    else
    {
        if (_check_bt_addr_valid(bt_addr_vram.bt_addr) == TRUE)
        {
            ret_val = 1;
        }
        else
        {
            ret_val = 0;
        }
    }

    if (ret_val == 1)
    {
        for (i = 0; i < 6; i++)
        {
            cmd_data[i] = bt_addr_vram.bt_addr[i];
        }

        print_log("bt addr: %x:%x:%x:%x:%x:%x\n", cmd_data[5], cmd_data[4], cmd_data[3], cmd_data[2], cmd_data[1],
                cmd_data[0]);
    }
    else
    {
        libc_memset(cmd_data, 0, sizeof(cmd_data));
    }

    if (g_test_mode != TEST_MODE_CARD)
    {
        return_data = (return_result_t *) (STUB_ATT_RETURN_DATA_BUFFER);

        return_data->test_id = TESTID_READ_BTADDR;

        return_data->test_result = ret_val;

        trans_bytes = 0;

        //蓝牙地址转unicode形式
        bytes_to_unicode(cmd_data, 5, 6, return_data->return_arg, &trans_bytes);

        //添加结束符
        return_data->return_arg[trans_bytes++] = 0x0000;

        //如果参数未四字节对齐，要四字节对齐处理
        if ((trans_bytes % 2) != 0)
        {
            return_data->return_arg[trans_bytes++] = 0x0000;
        }

        act_test_report_result(return_data, trans_bytes * 2 + 4);
    }

    return ret_val;
}

static int cmp_bt_name(uint8 *bt_name0, uint8 *bt_name1, uint32 cmp_len)
{
    uint32 i;

    for (i = 0; i < cmp_len; i++)
    {
        if (bt_name0[i] != bt_name1[i])
        {
            break;
        }

        if (bt_name0[i] == '\0')
        {
            return TRUE;
        }
    }

    if (i < cmp_len)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
int act_test_read_bt_name(void *arg_buffer)
{
    uint32 i;

    uint32 ret_val;

    read_btname_test_arg_t *read_btname_arg;

    nvram_param_rw_t param_rw;

    bt_addr_vram_t bt_addr_vram;

    return_result_t *return_data;

    uint32 trans_bytes;

    read_btname_arg = (read_btname_test_arg_t *) arg_buffer;

    sys_vm_read(&bt_addr_vram, VM_BTSTACK, sizeof(bt_addr_vram));

    if (bt_addr_vram.magic != VRAM_MAGIC(VM_BTSTACK))
    {
        param_rw.logical_index = PARAM_BT_NAME;
        param_rw.rw_len = sizeof(bt_addr_vram.device_name);
        param_rw.rw_buffer = &(bt_addr_vram.device_name[0]);

        //读取蓝牙名称
        ret_val = base_param_read(&param_rw);

        if (ret_val == 0)
        {
            if (bt_addr_vram.device_name[0] != 0)
            {
                ret_val = TRUE;
            }
            else
            {
                ret_val = FALSE;
            }
        }
        else
        {
            ret_val = FALSE;
        }

        if (ret_val == TRUE)
        {
            param_rw.logical_index = PARAM_BT_BLENAME;
            param_rw.rw_len = sizeof(bt_addr_vram.ble_device_name);
            param_rw.rw_buffer = &(bt_addr_vram.ble_device_name[0]);

            //读取蓝牙BLE名称
            ret_val = base_param_read(&param_rw);

            if (ret_val == 0)
            {
                if (bt_addr_vram.ble_device_name[0] != 0)
                {
                    ret_val = TRUE;
                }
                else
                {
                    ret_val = FALSE;
                }
            }
            else
            {
                ret_val = FALSE;
            }
        }
    }
    else
    {
        ret_val = TRUE;
    }

    if (ret_val == TRUE)
    {
        print_log("read bt name: %s\n", bt_addr_vram.device_name);

        print_log("read ble name: %s\n", bt_addr_vram.ble_device_name);
    }

    if (read_btname_arg->cmp_btname_flag == TRUE)
    {
        ret_val = cmp_bt_name(bt_addr_vram.device_name, read_btname_arg->cmp_btname, sizeof(bt_addr_vram.device_name));
    }

    if (ret_val == TRUE)
    {
        if (read_btname_arg->cmp_blename_flag == TRUE)
        {
            ret_val = cmp_bt_name(bt_addr_vram.ble_device_name, read_btname_arg->cmp_blename,
                    sizeof(bt_addr_vram.ble_device_name));
        }

    }

    if (g_test_mode != TEST_MODE_CARD)
    {
        return_data = (return_result_t *) (STUB_ATT_RETURN_DATA_BUFFER);

        return_data->test_id = TESTID_READ_BTNAME;

        return_data->test_result = ret_val;

        act_test_report_result(return_data, 4);
    }

    return ret_val;
}
