/********************************************************************************
 *        Copyright(c) 2014-2015 Actions (Zhuhai) Technology Co., Limited,
 *                            All Rights Reserved.
 *
 * 描述：音量管理接口，包括设置音量，切换音量模式，静音。
 * 作者：cailizhen
 ********************************************************************************/

#include "psp_includes.h"
#include "common_func.h"

/******************************************************************************/
/*!
 * \par  Description:
 *    获取当前声音音量。
 * \param[in]    none
 * \param[out]   none
 * \return       uint8 当前音量值，0xff表示无效
 * \note
 *******************************************************************************/
uint8 com_get_sound_volume(void)
{
    return sys_comval->volume_current;
}

/******************************************************************************/
/*!
 * \par  Description:
 *    重置当前声音音量。
 * \param[in]    none
 * \param[out]   none
 * \return       bool
 * \retval           TRUE 重置音量成功
 * \retval           FALSE 重置音量失败
 * \note
 *******************************************************************************/
bool com_reset_sound_volume(uint8 set_mode)
{
    sys_comval->volume_current_phy = 0xff;

    return com_set_sound_volume(sys_comval->volume_current, set_mode);
}

/******************************************************************************/
/*!
 * \par  Description:
 *    设置声音音量。
 * \param[in]    set_vol 设置音量值
 * \param[out]   none
 * \return       bool
 * \retval           TRUE 设置音量成功
 * \retval           FALSE 设置音量失败
 * \note
 *******************************************************************************/
bool com_set_sound_volume(uint8 set_vol, uint8 set_mode)
{
    bool bret = TRUE;

    sys_comval->volume_current = set_vol;
#pragma __PRQA_IGNORE_START__
    if (sys_comval->volume_mode >= VOLUME_MODE_MAX)
    {
        PRINT_WARNING("Invalid volume mode!");
    }
    else
    {
        sys_comval->volume[sys_comval->volume_mode] = set_vol;
    }
#pragma __PRQA_IGNORE_END__

    if (get_mute_enable() == FALSE)
    {


        if ((g_app_info_vector[APP_TYPE_BTSTACK].used == 1)
                && ((set_mode & SET_VOLUME_SYNC_TO_REMOTE) != 0))
        {
            if (g_this_app_info->app_id == APP_ID_BTPLAY)
            {
                com_btmanager_avrcp_update_volume_to_phone(set_vol);
            }
            else if (g_this_app_info->app_id == APP_ID_BTCALL)
            {
                com_btmanager_hfp_update_volume_to_phone(set_vol);
            }
            else
            {
                ; //nothing for QAC
            }
        }
        if (g_app_info_state.inner_pa_inited == TRUE)
        {
            com_set_phy_volume(set_vol);
        }       
    }

    return bret;
}

//切换音量模式
void com_switch_volume_mode(volume_mode_e mode)
{
    if (mode >= VOLUME_MODE_MAX)
    {
        PRINT_WARNING("invalid volume mode!!");
        return;
    }

    sys_comval->volume_current_phy = 0xff;
    sys_comval->volume_mode = mode;
    sys_comval->volume_current = sys_comval->volume[mode];
}

//设置音量负增益（衰减）
void com_set_volume_gain(uint8 gain)
{
    if (gain > VOLUME_TABLE_MAX)
    {
        PRINT_WARNING("Invalid volume gain!!");
        gain = VOLUME_TABLE_MAX;
    }

    sys_comval->volume_gain = gain;
}

/******************************************************************************/
/*!
 * \par  Description:
 *    启动或关闭静音。
 * \param[in]    mute TRUE表示静音，FALSE表示释放静音
 * \param[out]   none
 * \return       bool
 * \retval           TRUE 设置静音成功。
 * \retval           FALSE 设置静音失败。
 * \ingroup      common_func
 * \note
 *******************************************************************************/
bool com_set_mute(bool mute)
{
    sys_comval->volume_current_phy = 0xff;

    if (mute == TRUE)
    {
        if (get_mute_enable() == FALSE) //当前为非静音状态
        {
            //等待按键音结束，这样才能完整听到按键音
            keytone_play_deal_wait();
            //设置PA音量为0，置为静音状态
            com_set_phy_volume(0); //直接调用 com_set_phy_volume 设置，以防止修改了 volume_current
            change_mute_enable(TRUE);
        }
    }
    else
    {
        if (get_mute_enable() == TRUE) //当前为静音状态
        {
            //置为非静音状态，并设置PA音量
            change_mute_enable(FALSE);
            com_reset_sound_volume(0);
        }
    }

    return TRUE;
}

//静音状态转换
bool com_switch_mute(void)
{
    if (get_mute_enable() == TRUE) //当前为静音状态
    {
        com_set_mute(FALSE);
    }
    else //当前为非静音状态
    {
        com_set_mute(TRUE);
    }

    return TRUE;
}
