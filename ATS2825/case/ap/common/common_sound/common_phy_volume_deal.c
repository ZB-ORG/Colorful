/********************************************************************************
 *        Copyright(c) 2014-2015 Actions (Zhuhai) Technology Co., Limited,
 *                            All Rights Reserved.
 *
 * 描述：物理音量管理接口，包括设置音量和获取音量；支持I2S。
 * 作者：cailizhen
 ********************************************************************************/

#include "psp_includes.h"
#include "music_common.h"
#include "common_func.h"
#include "ccd_interface.h"

#if ((CASE_LINEIN_CHANNEL_SEL == 0) || (CASE_FM_CHANNEL_SEL == 0))
extern const int16 g_ana_pa_db[ANA_PA_VOLUME_MAX];
extern const vol_hard_t g_hard_volume_table[VOLUME_TABLE_MAX+1][VOLUME_VALUE_MAX+1];
#else
extern const vol_hard_t g_hard_volume_table[VOLUME_VALUE_MAX+1];
extern const vol_hard_t g_hard_volume_table_sm[VOLUME_VALUE_MAX+1];
#endif
extern const uint16 g_hard_volume_table_i2s_pa[VOLUME_VALUE_MAX+1];

static void _calc_keytone_precut_i2s_pa(uint8 set_vol, uint16 reg_val);
static void _calc_keytone_precut_inner_pa(uint8 set_vol, uint32 vol_hard_pa, uint32 vol_hard_da);
static void _set_phy_volume_i2s_pa(uint16 reg_val_to);
static void _set_phy_volume_inner_pa(uint32 vol_hard_pa_to, uint32 vol_hard_da_to);
uint32 __section__(".BANK46") mdrc_comp_handle(uint32 vol_hard_da);
static void _set_phy_volume_inner_pa_sm(uint32 vol_hard_pa_to, uint32 vol_hard_da_to);
static void _set_ao_source_dac_sm(int tab_index,uint8 vol_from, uint8 vol_to);



/******************************************************************************/
/*!
 * \par  Description:
 * \设置硬件音量调节
 * \param[in]    none
 * \param[out]   none
 * \return       none
 * \ingroup      common_func
 * \note
 * \li
 *******************************************************************************/
void com_set_phy_volume(uint8 set_vol)
{
    int tab_index, tab_index_tmp;
    int db;
    uint8 vol_from = sys_comval->volume_current_phy;
    uint8 vol_to = set_vol;
    bool vbass_enable_temp = sys_comval->dae_cfg.vbass_enable;

    PRINT_INFO_INT("set_vol",set_vol);

    if ((sys_comval->signal_energy_enable == 1)
        && (sys_comval->signal_variable_mode == 1)
        && (g_app_info_state_all.energy_available_flag == TRUE))
    {
        sys_comval->signal_energy = *(g_app_info_state_all.p_energy_value - 1);
        
        if(sys_comval->signal_energy<-300)
        {
            sys_comval->dae_cfg.vbass_enable = 0;        
        }
        else
        {
            //大信号的时候，通过APP设置过音效，就不做处理;否则恢复默认状态
            if (sys_comval->set_dae_by_app_flag == 1)
            {
                ;//for qac
            }
            else
            {
                sys_comval->dae_cfg.vbass_enable = sys_comval->default_vbass_enable;
            }
        }
    }
        
    if (sys_comval->volume_gain > VOLUME_TABLE_MAX)
    {
        PRINT_WARNING("Invalid volume gain!!");
        sys_comval->volume_gain = VOLUME_TABLE_MAX;
    }

    if (vol_to > VOLUME_VALUE_MAX)
    {
        vol_to = VOLUME_VALUE_MAX;
    }

    sys_comval->volume_current_phy = set_vol;
    if (STANDARD_MODE == sys_comval->dae_cfg.audiopp_type)
    {      
        if (vol_to == VOLUME_VALUE_MAX)
        {
            tab_index = 0;
        }
        else if (0 == vol_to)
        {
            tab_index = 31;
        }
        else
        {
            tab_index = 30 - vol_to + 2;
        }
    }
    else  if (SMART_MODE == sys_comval->dae_cfg.audiopp_type)
    {       
        tab_index = VOLUME_VALUE_MAX - vol_to;
    }

    //tab_index = VOLUME_VALUE_MAX - vol_to;

    switch (g_app_info_state.aout_mode)
    {
        case AO_SOURCE_I2S: //i2s
        //case AO_SOURCE_SPDIF: //spdif
        {
            uint16 reg_val_to;

            reg_val_to = g_hard_volume_table_i2s_pa[tab_index];

            //音量限制处理
            db = (int16) g_hard_volume_table_i2s_pa[0] - (int16) reg_val_to; //TO MODIFY CASE BY CASE
            db = db * 10 / 8; //TO MODIFY CASE BY CASE
            if (db > sys_comval->volume_limit)
            {
                reg_val_to = (uint16)(reg_val_to + ((db - sys_comval->volume_limit) * 8 / 10)); //TO MODIFY CASE BY CASE
            }

            _calc_keytone_precut_i2s_pa(vol_to, reg_val_to);

            if ((sys_comval->dae_cfg_variable == 1) && (vol_from != 0xff))
            {
                uint16 reg_val_from;
                uint16 tmp_reg_val_to;
                int16 tmp_step;
                bool last_set_flag = TRUE;

                if (vol_from > VOLUME_VALUE_MAX)
                {
                    vol_from = VOLUME_VALUE_MAX;
                }
                tab_index = VOLUME_VALUE_MAX - vol_from;

                reg_val_from = g_hard_volume_table_i2s_pa[tab_index];

                PRINT_DBG_INT("val_from:", reg_val_from);
                PRINT_DBG_INT("val_to:", reg_val_to);

                if (reg_val_from < reg_val_to)
                {
                    if ((reg_val_from + 0x20) >= reg_val_to) //TO MODIFY CASE BY CASE
                    {
                        tmp_step = 4; //TO MODIFY CASE BY CASE
                    }
                    else
                    {
                        tmp_step = (int16)((reg_val_to - reg_val_from) / 8);
                    }

                    tmp_reg_val_to = reg_val_from + tmp_step;
                    for (; tmp_reg_val_to <= reg_val_to; tmp_reg_val_to += tmp_step)
                    {
                        PRINT_DBG_INT("tmp_to:", tmp_reg_val_to);
                        _set_phy_volume_i2s_pa(tmp_reg_val_to);
                        if (tmp_reg_val_to == reg_val_to)
                        {
                            last_set_flag = FALSE;
                            break;
                        }
                    }
                    if (last_set_flag == TRUE)
                    {
                        PRINT_DBG_INT("tmp_to:", reg_val_to);
                        _set_phy_volume_i2s_pa(reg_val_to);
                    }
                    break;
                }
                else if (reg_val_from > reg_val_to)
                {
                    if (reg_val_from <= (reg_val_to + 0x20)) //TO MODIFY CASE BY CASE 大概为4db
                    {
                        tmp_step = 4; //TO MODIFY CASE BY CASE 大概步进为0.5db
                    }
                    else
                    {
                        tmp_step = (int16)((reg_val_from - reg_val_to) / 8);
                    }

                    tmp_reg_val_to = reg_val_from - tmp_step;
                    for (; tmp_reg_val_to >= reg_val_to; tmp_reg_val_to -= tmp_step)
                    {
                        PRINT_DBG_INT("tmp_to:", tmp_reg_val_to);
                        _set_phy_volume_i2s_pa(tmp_reg_val_to);

                        if (tmp_reg_val_to == reg_val_to)
                        {
                            last_set_flag = FALSE;
                            break;
                        }
                        if ((tmp_reg_val_to - reg_val_to) < tmp_step) //规避算术运算溢出问题
                        {
                            break;
                        }
                    }
                    if (last_set_flag == TRUE)
                    {
                        PRINT_DBG_INT("tmp_to:", reg_val_to);
                        _set_phy_volume_i2s_pa(reg_val_to);
                    }
                    break;
                }
                else
                {
                    ;//for qac
                }
            }

            _set_phy_volume_i2s_pa(reg_val_to);
        }
        break;

        //internel dac & i2s or spdif and external dac both chanel
        case AO_SOURCE_DACI2S: //i2s + internel dac
        case AO_SOURCE_DACSPDIF: //spdif +internel dac
        default:
        {
            if (SMART_MODE == sys_comval->dae_cfg.audiopp_type)
            {              
                uint32 vol_hard_pa_to, vol_hard_da_to;
                uint32 vol_hard_pa_tmp, vol_hard_da_tmp;
               
#if ((CASE_LINEIN_CHANNEL_SEL == 0) || (CASE_FM_CHANNEL_SEL == 0))
                vol_hard_pa_to = g_hard_volume_table[sys_comval->volume_gain][tab_index].vol_pa;
                vol_hard_da_to = g_hard_volume_table[sys_comval->volume_gain][tab_index].vol_da;

                //音量限制处理
                vol_hard_pa_tmp = g_hard_volume_table[sys_comval->volume_gain][0].vol_pa;
                vol_hard_da_tmp = g_hard_volume_table[sys_comval->volume_gain][0].vol_da;

                db = g_ana_pa_db[vol_hard_pa_to] - g_ana_pa_db[vol_hard_pa_tmp];
                db += ((int32) vol_hard_da_to - (int32) vol_hard_da_tmp) * 15 / 4;
#else
                if (VOLUME_VALUE_MAX == tab_index)
                {
                    vol_hard_pa_to = 0;
                }
                else
                {
                    vol_hard_pa_to = g_hard_volume_table[tab_index].vol_pa - sys_comval->volume_gain;
                }
                vol_hard_da_to = g_hard_volume_table[tab_index].vol_da;

                //音量限制处理
                vol_hard_pa_tmp = g_hard_volume_table[0].vol_pa - sys_comval->volume_gain;
                vol_hard_da_tmp = g_hard_volume_table[0].vol_da;

                db = ((int16) vol_hard_da_to - (int16) vol_hard_da_tmp) * 15 / 4;
#endif
                //音量限制处理
                if (db > sys_comval->volume_limit)
                {
                    vol_hard_da_to = vol_hard_da_to - (uint32)((db - sys_comval->volume_limit) * 4 / 15);
                }

                _calc_keytone_precut_inner_pa(vol_to, vol_hard_pa_to, vol_hard_da_to);
#if ((CASE_LINEIN_CHANNEL_SEL == 1) && (CASE_FM_CHANNEL_SEL == 1))
                    if ((sys_comval->dae_cfg_variable == 1) && (vol_from != 0xff))
                    {
                        uint32 vol_hard_da_from;
                        uint32 tmp_vol_hard_da_to;
                        int16 tmp_step;
                        bool last_set_flag = TRUE;

                        if (vol_from > VOLUME_VALUE_MAX)
                        {
                            vol_from = VOLUME_VALUE_MAX;
                        }
                        tab_index = VOLUME_VALUE_MAX - vol_from;

                        vol_hard_da_from = g_hard_volume_table[tab_index].vol_da;

                        if (vol_hard_da_from < vol_hard_da_to)
                        {
                            if ((vol_hard_da_from + 10) >= vol_hard_da_to)
                            {
                                tmp_step = 2;
                            }
                            else if ((vol_hard_da_from + 26) >= vol_hard_da_to)
                            {
                                tmp_step = 1;
                            }
                            else if ((vol_hard_da_from + 28) >= vol_hard_da_to)
                            {
                                tmp_step = 2;
                            }
                            else
                            {
                                tmp_step = (int16)((vol_hard_da_to - vol_hard_da_from) / 5);
                            }

                            tmp_vol_hard_da_to = vol_hard_da_from + (uint32)tmp_step;
                            for (; tmp_vol_hard_da_to <= vol_hard_da_to; tmp_vol_hard_da_to += (uint32)tmp_step)
                            {
                                _set_phy_volume_inner_pa(vol_hard_pa_to, tmp_vol_hard_da_to);
                                if (tmp_vol_hard_da_to == vol_hard_da_to)
                                {
                                    last_set_flag = FALSE;
                                    break;
                                }
                            }
                            if (last_set_flag == TRUE)
                            {
                                _set_phy_volume_inner_pa(vol_hard_pa_to, vol_hard_da_to);
                            }
                            break;
                        }
                        else if (vol_hard_da_from > vol_hard_da_to)
                        {
                            if (vol_hard_da_from <= (vol_hard_da_to + 10))
                            {
                                tmp_step = 2;
                            }
                            else if (vol_hard_da_from <= (vol_hard_da_to + 26))
                            {
                                tmp_step = 1;
                            }
                            else if (vol_hard_da_from <= (vol_hard_da_to + 28))
                            {
                                tmp_step = 2;
                            }
                            else
                            {
                                tmp_step = (int16)((vol_hard_da_from - vol_hard_da_to) / 5);
                            }

                            tmp_vol_hard_da_to = (uint32)vol_hard_da_from - (uint32)tmp_step;
                            for (; tmp_vol_hard_da_to >= vol_hard_da_to; tmp_vol_hard_da_to -= (uint32)tmp_step)
                            {
                                _set_phy_volume_inner_pa(vol_hard_pa_to, tmp_vol_hard_da_to);

                                if (tmp_vol_hard_da_to == vol_hard_da_to)
                                {
                                    last_set_flag = FALSE;
                                    break;
                                }
                                if ((tmp_vol_hard_da_to - vol_hard_da_to) < (uint32)tmp_step) //规避算术运算溢出问题
                                {
                                    break;
                                }
                            }
                            if (last_set_flag == TRUE)
                            {
                                _set_phy_volume_inner_pa(vol_hard_pa_to, vol_hard_da_to);
                            }
                            break;
                        }
                        else
                        {
                            ;//for qac
                        }
                    }
#endif
                    _set_phy_volume_inner_pa(vol_hard_pa_to, vol_hard_da_to);       
                }
            else if (STANDARD_MODE == sys_comval->dae_cfg.audiopp_type)
            {
                _set_ao_source_dac_sm(tab_index, vol_from, vol_to);
            }

        }
        break;
    }
    sys_comval->dae_cfg.vbass_enable = vbass_enable_temp;
}
void com_update_volume_limit(int8 vol_limit)
{
    sys_comval->volume_limit = vol_limit;
    if(get_mute_enable() == FALSE)//静音时不能调音量
    {
        com_set_phy_volume(sys_comval->volume_current);
    }
}

static void _calc_keytone_precut_i2s_pa(uint8 set_vol, uint16 reg_val)
{
    int db;
    int tab_index_tmp;

    if (set_vol > sys_comval->kt_tts_max_volume)
    {
        tab_index_tmp = VOLUME_VALUE_MAX - sys_comval->kt_tts_max_volume;

        db = (int16) reg_val - (int16) g_hard_volume_table_i2s_pa[tab_index_tmp];
        db = db * 10 / 8; //TO MODIFY CASE BY CASE

        if (db >= 0)
        {
            sys_comval->kt_tts_percent = 100;
        }
        else
        {
            sys_comval->kt_tts_percent = (uint8)(com_math_exp_fixed(db));//db为负数
        }
    }
    else
    {
        sys_comval->kt_tts_percent = 100;
    }
}

static void _calc_keytone_precut_inner_pa(uint8 set_vol, uint32 vol_hard_pa, uint32 vol_hard_da)
{
    int db;
    int tab_index_tmp;
    uint32 vol_hard_pa_tmp, vol_hard_da_tmp;

#if ((CASE_LINEIN_CHANNEL_SEL == 0) || (CASE_FM_CHANNEL_SEL == 0))

    if (set_vol > sys_comval->kt_tts_max_volume)
    {
        tab_index_tmp = VOLUME_VALUE_MAX - sys_comval->kt_tts_max_volume;

        vol_hard_pa_tmp = g_hard_volume_table[sys_comval->volume_gain][tab_index_tmp].vol_pa;
        vol_hard_da_tmp = g_hard_volume_table[sys_comval->volume_gain][tab_index_tmp].vol_da;

        db = (g_ana_pa_db[vol_hard_pa_tmp] + ((int16) vol_hard_da_tmp * 15 / 4))
                - (g_ana_pa_db[vol_hard_pa] + ((int16) vol_hard_da * 15 / 4));

        if (db >= 0)
        {
            sys_comval->kt_tts_percent = 100;
        }
        else
        {
            sys_comval->kt_tts_percent = (uint8)(com_math_exp_fixed(db));//db为负数
        }
    }
    else
    {
        sys_comval->kt_tts_percent = 100;
    }

#else

    if (set_vol > sys_comval->kt_tts_max_volume)
    {
        tab_index_tmp = VOLUME_VALUE_MAX - sys_comval->kt_tts_max_volume;

        vol_hard_da_tmp = g_hard_volume_table[tab_index_tmp].vol_da;

        db = ((int16) vol_hard_da_tmp - (int16) vol_hard_da) * 15 / 4;

        if (db >= 0)
        {
            sys_comval->kt_tts_percent = 100;
        }
        else
        {
            sys_comval->kt_tts_percent = (uint8)(com_math_exp_fixed(db));//db为负数
        }
    }
    else
    {
        sys_comval->kt_tts_percent = 100;
    }

#endif
}

//注意：该接口需要根据PA音量单位修改，修改点注释为 //TO MODIFY CASE BY CASE
static void _set_phy_volume_i2s_pa(uint16 reg_val_to)
{
    uint16 reg_val = reg_val_to;
    int db;

    db = (int16) g_hard_volume_table_i2s_pa[0] - (int16) reg_val_to; //TO MODIFY CASE BY CASE
    db = db * 10 / 8; //TO MODIFY CASE BY CASE
    sys_comval->volume_relative = (int16)db;

    if (reg_val < 0x3ff) //TO MODIFY CASE BY CASE
    {
        reg_val += ((uint16) sys_comval->volume_gain * 8); //TO MODIFY CASE BY CASE
    }

    if (sys_comval->dae_cfg_variable == 1)
    {
        com_set_dae_config_dynamic();

        if (sys_comval->dae_cfg.mdrc_enable == 1)
        {
            int16 limit_threshold;
            int16 vol_offset, tmp_vol_offset;

            limit_threshold = sys_comval->dae_cfg.mdrc_threshold_base_max;
            if (sys_comval->dae_cfg.limiter_enable == 1)
            {
                limit_threshold += ((int16) sys_comval->dae_cfg.limiter_threshold_diff \
                    - sys_comval->dae_cfg.limiter_threshold);
            }

            vol_offset  = (int16) 0 - limit_threshold;
            vol_offset -= (int16)sys_comval->dae_cfg.makeup_gain;
            vol_offset += sys_comval->dae_cfg.precut_ratio;
            vol_offset += sys_comval->mdrc_vol_adjust;

            if (vol_offset > 0)
            {
                reg_val -= vol_offset * 8 / 10; //TO MODIFY CASE BY CASE
            }
            else
            {
                reg_val += ((int16) 0 - vol_offset) * 8 / 10; //TO MODIFY CASE BY CASE
            }

            //保证PA音量递增
            tmp_vol_offset = vol_offset * 8 / 10; //TO MODIFY CASE BY CASE
            tmp_vol_offset = tmp_vol_offset * 10 / 8; //TO MODIFY CASE BY CASE

            if (vol_offset != tmp_vol_offset)
            {
                reg_val--; //TO MODIFY CASE BY CASE
            }

            if (reg_val < g_hard_volume_table_i2s_pa[0]) //TO MODIFY CASE BY CASE
            {
                reg_val = g_hard_volume_table_i2s_pa[0];
            }
        }
    }

    ccd_i2s_pa_set_volume(reg_val);
}

static void _set_phy_volume_inner_pa(uint32 vol_hard_pa_to, uint32 vol_hard_da_to)
{
    uint32 vol_hard_pa, vol_hard_da;
    uint32 vol_hard_pa_tmp, vol_hard_da_tmp;
    int db;

    vol_hard_pa = vol_hard_pa_to;
    vol_hard_da = vol_hard_da_to;

#if ((CASE_LINEIN_CHANNEL_SEL == 0) || (CASE_FM_CHANNEL_SEL == 0))
    vol_hard_pa_tmp = g_hard_volume_table[sys_comval->volume_gain][0].vol_pa;
    vol_hard_da_tmp = g_hard_volume_table[sys_comval->volume_gain][0].vol_da;

    db = g_ana_pa_db[vol_hard_pa_to] - g_ana_pa_db[vol_hard_pa_tmp];
    db += ((int32) vol_hard_da_to - (int32) vol_hard_da_tmp) * 15 / 4;

    sys_comval->volume_relative = (int16)db;
#endif

#if ((CASE_LINEIN_CHANNEL_SEL == 1) && (CASE_FM_CHANNEL_SEL == 1))

    db = ((int16) vol_hard_da_to - (int16) g_hard_volume_table[0].vol_da) * 15 / 4;
    sys_comval->volume_relative = (int16)db;

    if (sys_comval->dae_cfg_variable == 1)
    {
        com_set_dae_config_dynamic();
        if(SMART_MODE == sys_comval->dae_cfg.audiopp_type)
        {
             if (sys_comval->dae_cfg.mdrc_enable == 1)
            {
                int16 limit_threshold;
                int16 vol_offset, tmp_vol_offset;

            limit_threshold = sys_comval->dae_cfg.mdrc_threshold_base_max;
            if (sys_comval->dae_cfg.limiter_enable == 1)
            {
                limit_threshold += ((int16) sys_comval->dae_cfg.limiter_threshold_diff \
                    - sys_comval->dae_cfg.limiter_threshold);
            }

            vol_offset  = (int16) 0 - limit_threshold;
            vol_offset -= (int16)sys_comval->dae_cfg.makeup_gain;
            vol_offset += sys_comval->dae_cfg.precut_ratio;
            vol_offset += sys_comval->mdrc_vol_adjust;

            if (vol_offset > 0)
            {
                vol_hard_da += (uint32)(vol_offset * 4 / 15);
            }
            else
            {
                vol_hard_da -= (uint32)(((int16) 0 - vol_offset) * 4 / 15);
            }

            //保证PA音量递增
            tmp_vol_offset = vol_offset * 4 / 15;
            tmp_vol_offset = tmp_vol_offset * 15 / 4;

            if (vol_offset != tmp_vol_offset)
            {
                vol_hard_da++;
            }

            if (vol_hard_da > g_hard_volume_table[0].vol_da)
            {
                vol_hard_da = g_hard_volume_table[0].vol_da;
            }
            }
        }
        else if(STANDARD_MODE == sys_comval->dae_cfg.audiopp_type)
        {
           vol_hard_da = mdrc_comp_handle(vol_hard_da);
        }
        
    }

#endif
    while(set_pa_volume(vol_hard_pa, vol_hard_da) != 0)
    {
        sys_os_time_dly(10);
    }
}

//小米模型DAC输出 音量处理
static void _set_ao_source_dac_sm(int tab_index,uint8 vol_from, uint8 vol_to)
{
    uint32 vol_hard_pa_to, vol_hard_da_to;  
    uint32 vol_hard_pa_tmp, vol_hard_da_tmp;
    int db;
    
    if (VOLUME_VALUE_MAX == tab_index)
    {
        vol_hard_pa_to = 0;
    }
    else
    {
        vol_hard_pa_to = g_hard_volume_table_sm[tab_index].vol_pa - sys_comval->volume_gain;
    }
    vol_hard_da_to = g_hard_volume_table_sm[tab_index].vol_da;

    //音量限制处理
    vol_hard_pa_tmp = g_hard_volume_table_sm[0].vol_pa - sys_comval->volume_gain;
    vol_hard_da_tmp = g_hard_volume_table_sm[0].vol_da;

    db = ((int16) vol_hard_da_to - (int16) vol_hard_da_tmp) * 15 / 4;
    
    //音量限制处理
    if (db > sys_comval->volume_limit)
    {
        vol_hard_da_to = vol_hard_da_to - (uint32)((db - sys_comval->volume_limit) * 4 / 15);
    }

    _calc_keytone_precut_inner_pa(vol_to, vol_hard_pa_to, vol_hard_da_to);
    
    if (vol_from != 0xff)
    {
         _set_phy_volume_inner_pa_sm(vol_hard_pa_to, vol_hard_da_to);
    }
}

static void _set_phy_volume_inner_pa_sm(uint32 vol_hard_pa_to, uint32 vol_hard_da_to)
{
    uint32 vol_hard_pa, vol_hard_da;
    int db;

    vol_hard_pa = vol_hard_pa_to;
    vol_hard_da = vol_hard_da_to;

#if ((CASE_LINEIN_CHANNEL_SEL == 1) && (CASE_FM_CHANNEL_SEL == 1))

    db = ((int16) vol_hard_da_to - (int16) g_hard_volume_table_sm[0].vol_da) * 15 / 4;
    sys_comval->volume_relative = (int16)db;

    if (sys_comval->dae_cfg_variable == 1)
    {
        com_set_dae_config_dynamic();
        
        if(STANDARD_MODE == sys_comval->dae_cfg.audiopp_type)
        {
           vol_hard_da = mdrc_comp_handle(vol_hard_da);
        }        
    }

#endif

    while(set_pa_volume(vol_hard_pa, vol_hard_da) != 0)
    {
        sys_os_time_dly(10);
    }
}

uint32 __section__(".BANK46") mdrc_comp_handle(uint32 vol_hard_da)
{
        
       uint32 vol_hard_da_temp = 0;

       vol_hard_da_temp = vol_hard_da;
             
       if (sys_comval->dae_cfg.mdrc_enable_standard_mode == 1)
        {
            int16 compressor_threshold;
            int16 vol_offset, tmp_vol_offset;
       
            vol_offset = 0;

            vol_offset += sys_comval->mdrc_vol_adjust_standard_mode;            

            if (vol_offset > 0)
            {
                vol_hard_da_temp += (uint32)(vol_offset * 4 / 15);
            }
            else
            {
                vol_hard_da_temp -= (uint32)(((int16) 0 - vol_offset) * 4 / 15);
            }

            //保证PA音量递增
            tmp_vol_offset = vol_offset * 4 / 15;
            tmp_vol_offset = tmp_vol_offset * 15 / 4;

            if (vol_offset != tmp_vol_offset)
            {
                vol_hard_da_temp++;
            }

            if (vol_hard_da_temp > 0xbf)
            {
                vol_hard_da_temp = 0xbf;
            }
        }
  
       return vol_hard_da_temp;
}

/*! \endcond */

