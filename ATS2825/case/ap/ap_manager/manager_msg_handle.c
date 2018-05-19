/*******************************************************************************
 *                              US212A
 *                            Module: Manager
 *                 Copyright(c) 2003-2012 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>    <time>           <version >             <desc>
 *       zhangxs     2011-09-05 10:00     1.0             build this file
 *******************************************************************************/
/*!
 * \file     manager_msg_handle.c
 * \brief    消息处理模块
 * \author   zhangxs
 * \version  1.0
 * \date  2011/9/05
 *******************************************************************************/
#include  "manager.h"

extern void manager_msg_callback_sub(private_msg_t *pri_msg);

/******************************************************************************/
/*!
 * \par  Description:
 *    消息处理
 * \param[in]    私有消息指针
 * \param[out]   none
 * \return       none
 * \retval
 * \retval
 * \ingroup      module name
 * \par          exmaple code
 * \code
 *               list example code in here
 * \endcode
 * \note
 *******************************************************************************/
void manager_msg_callback(private_msg_t *pri_msg)
{
    uint8 ap_name[12];
    uint8 app_id;
    uint8 ap_param;

    switch (pri_msg->msg.type)
    {
        //创建前台应用
        case MSG_CREAT_APP:
        {
            adjust_freq_set_level(AP_PROCESS_MANAGER_PRIO,FREQ_LEVEL7,FREQ_NULL);
            
            app_id = pri_msg->msg.content.data[0];
            ap_param = pri_msg->msg.content.data[1];
            
            wait_ap_exit();
    
            _get_app_name(ap_name, app_id, APP_TYPE_GUI);
            
            sys_free_ap(FALSE);
            sys_exece_ap(ap_name, 0, (int32) ap_param);
            
            adjust_freq_set_level(AP_PROCESS_MANAGER_PRIO,FREQ_LEVEL3,FREQ_NULL);
        }
        break;

        //创建后台应用
        case MSG_CREAT_APP_SYNC:
        {
            app_id = pri_msg->msg.content.data[0];
            ap_param = pri_msg->msg.content.data[1];
            
            //应用约定如果另一个后台存在，需要先把当前后台杀死，然后再创建需要的后台
            if (g_app_info_vector[APP_TYPE_CONSOLE].used == 0)
            {
                _get_app_name(ap_name, app_id, APP_TYPE_CONSOLE);
                
                //wait for cur app exit
                libc_waitpid(0, 1);
    
                sys_free_ap(1);
                if (sys_exece_ap(ap_name, 1, (int32) ap_param) >= 0)
                {
                    pri_msg->reply->type = MSG_REPLY_SUCCESS;
                }
                else
                {
                    pri_msg->reply->type = MSG_REPLY_FAILED;
                }
                libc_sem_post(pri_msg->sem);
            }
            else
            //如果后台存在才处理
            {
                pri_msg->reply->type = MSG_REPLY_SUCCESS;
                libc_sem_post(pri_msg->sem);
            }
        }
        break;

        case MSG_KILL_APP_SYNC:
        //如果后台存在才处理
        {
            app_id = pri_msg->msg.content.data[0];
            ap_param = pri_msg->msg.content.data[1];
            if (app_id == APP_ID_THEENGINE)
            {
                app_id = get_engine_appid_by_type(get_engine_type());
            }
            
            if (app_id == APP_ID_MAX)
            {
                goto kill_over;
            }
            
            if (((app_id == APP_ID_BTSTACK) && (g_app_info_vector[APP_TYPE_BTSTACK].used != 0))
             || ((app_id != APP_ID_BTSTACK) && (g_app_info_vector[APP_TYPE_CONSOLE].used != 0)))
            {
                //杀死后台
                kill_app(app_id, ap_param);
                if (app_id == APP_ID_BTSTACK)
                {
                    sys_free_ap(AP_BTSTACK_FILE);
                }
                else
                {
                    sys_free_ap(1);
                }
            }
            
            //如果应用已经不存在，直接返回成功，避免前台收不到响应而死等
            kill_over:
            pri_msg->reply->type = MSG_REPLY_SUCCESS;
            libc_sem_post(pri_msg->sem);
        }
        break;

        default:
        manager_msg_callback_sub(pri_msg);
        break;
    }

}
