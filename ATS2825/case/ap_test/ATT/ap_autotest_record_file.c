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

#pragma __PRQA_IGNORE_START__

static uint8 file_name_buffer[80] _BANK_DATA_ATTR_;

#if 0
const uint8 bt_name_tips_ok[] = "modify bt name OK";
const uint8 bt_name_tips_fail[] = "modify bt name FAILED";

const uint8 ble_name_tips_ok[] = "modify ble name OK";
const uint8 ble_name_tips_fail[] = "modify ble name FAILED";

const uint8 bt_addr_tips_ok[] = "modify bt addr OK";
const uint8 bt_addr_tips_fail[] = "modify bt addr FAILED";
#endif

static void create_file_name(uint8 *file_name, uint32 record_count)
{
    int i = 0;
    date_t cur_date;
    time_t cur_time;

    file_name[i++] = 0xff;
    file_name[i++] = 0xfe;

    file_name[i++] = 'A';
    file_name[i++] = 0x0;

    file_name[i++] = 'T';
    file_name[i++] = 0x0;

    file_name[i++] = 'T';
    file_name[i++] = 0x0;

    file_name[i++] = '_';
    file_name[i++] = 0x0;

    //读取日期和时间
    sys_get_date(&cur_date);

    //显示year
    file_name[i++] = ((cur_date.year%10000)/1000) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = ((cur_date.year%1000)/100) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = ((cur_date.year%100)/10) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = (cur_date.year%10) + '0';
    file_name[i++] = 0x0;

    //显示month
    file_name[i++] = (cur_date.month / 10) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = (cur_date.month % 10) + '0';
    file_name[i++] = 0x0;

    //显示day
    file_name[i++] = (cur_date.day / 10) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = (cur_date.day % 10) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = '_';
    file_name[i++] = 0x0;

    sys_get_time(&cur_time);

    //显示小时
    file_name[i++] = (cur_time.hour / 10) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = (cur_time.hour % 10) + '0';
    file_name[i++] = 0x0;

    //显示分钟
    file_name[i++] = (cur_time.minute / 10) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = (cur_time.minute % 10) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = '_';
    file_name[i++] = 0x0;

    //显示count
    file_name[i++] = (record_count/1000000) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = ((record_count%1000000)/100000) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = ((record_count%100000)/10000) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = ((record_count%10000)/1000) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = ((record_count%1000)/100) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = ((record_count%100)/10) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = (record_count%10) + '0';
    file_name[i++] = 0x0;

    file_name[i++] = 0x2e;
    file_name[i++] = 0x0;

    file_name[i++] = 'L';
    file_name[i++] = 0x0;

    file_name[i++] = 'O';
    file_name[i++] = 0x0;

    file_name[i++] = 'G';
    file_name[i++] = 0x0;

    file_name[i++] = 0x0;
    file_name[i++] = 0x0;
}

int32 create_record_file(void)
{
    int ret_val;
    int32 file_handle;
    uint8 *file_buffer = (uint8 *)LOG_FILE_BUFFER;

    //定位到根目录
    vfs_cd(g_file_sys_id, CD_ROOT, 0);

    ret_val = vfs_file_dir_exist(g_file_sys_id, file_name_buffer, 1);

    libc_memset(file_buffer, 0, LOG_FILE_LEN);

    if(ret_val == 0)
    {
        file_handle = vfs_file_create(g_file_sys_id, file_name_buffer, LOG_FILE_LEN);

        if(file_handle == 0)
        {
            libc_print("create recrod file error!\n", 0, 0);

            while(1);
        }

        vfs_file_write(g_file_sys_id, file_buffer, LOG_FILE_LEN, file_handle);

        vfs_file_close(g_file_sys_id, file_handle);
    }
    else
    {
        file_handle = vfs_file_open(g_file_sys_id, file_name_buffer, OPEN_MODIFY);

        if(file_handle == 0)
        {
            libc_print("open file error!\n", 0, 0);

            while(1);
        }

        vfs_file_write(g_file_sys_id, file_buffer, LOG_FILE_LEN, file_handle);

        vfs_file_close(g_file_sys_id, file_handle);
    }

    return file_handle;
}

void close_log_file(uint32 file_handle)
{
    uint8 *file_buffer = (uint8 *)ATT_LOG_FILE_BUFFER;

    vfs_file_write(g_file_sys_id, file_buffer, g_write_file_len, file_handle);

    DEBUG_ATT_PRINT("write file over!\n", 0, 0);

    vfs_file_close(g_file_sys_id, file_handle);

    return;
}



void write_log_file(void)
{
    int32 file_handle;
    int32 record_count;

    sys_os_sched_lock();

    record_count = read_att_test_count();

    //创建文件名
    create_file_name(file_name_buffer, record_count);

    create_record_file();

    file_handle = vfs_file_open(g_file_sys_id, file_name_buffer, OPEN_MODIFY);

    close_log_file(file_handle);

    act_test_write_att_record_file(0, ++record_count, 1);

    DEBUG_ATT_PRINT("write log file ok!\n", 0, 0);

    sys_os_sched_unlock();

    led_flash_ok();

    return;
}
#pragma __PRQA_IGNORE_END__


