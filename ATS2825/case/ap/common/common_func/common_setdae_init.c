/********************************************************************************
 *        Copyright(c) 2014-2015 Actions (Zhuhai) Technology Co., Limited,
 *                            All Rights Reserved.
 *
 * ������CASE DAE���ù���ģ�飬��������Ĭ��ֵ����ʼ�������ýӿڣ�ͨ�������ڴ����
 *       ������á�
 * ���ߣ�cailizhen
 ********************************************************************************/

#include "psp_includes.h"
#include "music_common.h"
#include "common_func.h"
#include "stub_command.h"
#include "../aset_common/common_aset_new_func.h"

void load_standard_mode_mdrc_param(dae_config_t *p_dae_cfg, bool aux_flag);

//��config.bin����DAEĬ��������
void com_load_dae_config(comval_t *setting_comval)
{
    dae_config_t *p_dae_cfg;
    uint8 i;
    dae_compressor_temp_standard_mode_t    compressor_temp_standard_mode;

    p_dae_cfg = &(setting_comval->dae_cfg);
            
    //��Ч������ 
    p_dae_cfg->audiopp_type = (uint8) com_get_config_default(DAE_DEFAULT_TYPE);  
    
    p_dae_cfg->compressor_enable_standard_mode = (uint8) com_get_config_default(DAE_COMPRESSOR_ENABLE); 
    com_get_config_struct((uint16) (DAE_COMPRESSOR_SETTING), (uint8 *)&compressor_temp_standard_mode ,  \
        sizeof(dae_compressor_temp_standard_mode_t));
    libc_memcpy(&(p_dae_cfg->compressor_standard_mode), &compressor_temp_standard_mode.compressor,      \
        sizeof(dae_compressor_standard_mode_t));
    
    p_dae_cfg->mdrc_enable_standard_mode = (uint8) com_get_config_default(DAE_MDRC_ENABLE_STANDARD_MODE);

    p_dae_cfg->precut_standard_mode = (int8) com_get_config_default(DAE_PRECUT_RATIO);
    p_dae_cfg->dynamic_precut_standard_mode = 0;
    
    p_dae_cfg->compressor_threshold_diff = 0;
    
    //CASE ���Զ��� Graphics EQ ����
    setting_comval->eq_enable = (uint8) com_get_config_default(DAE_GRAPHICS_EQ_ENABLE);
    setting_comval->eq_type   = (uint8) com_get_config_default(DAE_GRAPHICS_EQ_TYPE);

    setting_comval->highpass_cutoff = 0;
    setting_comval->aux_flag = FALSE;
    setting_comval->dae_cfg_variable = FALSE; 

#ifdef WAVES_ASET_TOOLS    
    p_dae_cfg->bypass = 1;   //ʹ��WAVES pc tools��actions dae bypassģʽ 
#else
    p_dae_cfg->bypass = (uint8) com_get_config_default(DAE_BYPASS_ENABLE);
#endif

    p_dae_cfg->equivalent_gain = 0;
    p_dae_cfg->precut_ratio = 0;
    p_dae_cfg->mdrc_precut_ratio = 0;
    p_dae_cfg->makeup_gain = 0;
    p_dae_cfg->post_precut_ratio = 0;

    p_dae_cfg->peq_enable = (uint8) com_get_config_default(DAE_PEQ_ENABLE);
    for (i = 0; i < MAX_PEQ_SEG; i++)
    {
        peq_config_t tmp_peq_config;

        com_get_config_struct((uint16) (DAE_PEQ_BANK0_SETTING + i), (uint8 *) &tmp_peq_config,          \
            sizeof(peq_config_t));
        libc_memcpy(&(p_dae_cfg->peq_bands[i]), &(tmp_peq_config.eq_data), sizeof(peq_band_t));

        if ((p_dae_cfg->peq_bands[i].type == 2) && (p_dae_cfg->peq_bands[i].gain != 0))
        {
            setting_comval->highpass_cutoff = (uint8)p_dae_cfg->peq_bands[i].cutoff;
        }

        p_dae_cfg->peq_band_enable_type[i] = (uint8)tmp_peq_config.default_value;
    }

    p_dae_cfg->spk_compensation_enable = (uint8) com_get_config_default(DAE_SPK_COMPENSATION_ENABLE);
    {
        spk_compensation_config_t tmp_spk_compensation_cfg;

        p_dae_cfg->spk_comp_source = 0;

        com_get_config_struct(DAE_SPK_COMPENSATION_SETTING, \
            (uint8 *) &tmp_spk_compensation_cfg, sizeof(spk_compensation_config_t));

        p_dae_cfg->spk_comp_filter_order = tmp_spk_compensation_cfg.filter_order;
        p_dae_cfg->spk_comp_filter_QvalLeft = (uint8)tmp_spk_compensation_cfg.filter_QvalLeft;
        p_dae_cfg->spk_comp_filter_QvalRight = (uint8)tmp_spk_compensation_cfg.filter_QvalRight;
    }

    setting_comval->signal_energy_enable = (uint8) com_get_config_default(DAE_SIGNAL_ENERGY_ENABLE);
    setting_comval->signal_energy_used = 0;
    setting_comval->signal_variable_mode = 0;
    {
        enhanced_signal_config_t tmp_enhanced_signal_cfg;

        com_get_config_struct(DAE_SIGNAL_ENERGY_SETTING, (uint8 *) &tmp_enhanced_signal_cfg,            \
            sizeof(enhanced_signal_config_t));

        p_dae_cfg->energy_detect_enable = 0;
        p_dae_cfg->signal_energy_init = 0;
        p_dae_cfg->period = tmp_enhanced_signal_cfg.signal_cfg.period;
        p_dae_cfg->period_count = (uint8)tmp_enhanced_signal_cfg.signal_cfg.period_count;
        p_dae_cfg->preadjust_db = tmp_enhanced_signal_cfg.signal_cfg.preadjust_db;
        p_dae_cfg->preadjust_count = (uint8)tmp_enhanced_signal_cfg.signal_cfg.preadjust_count;
        setting_comval->signal_energy_min = tmp_enhanced_signal_cfg.signal_cfg.signal_energy_min;
        setting_comval->signal_energy = 0;
    }

    setting_comval->dae_enhanced_enable = (uint8) com_get_config_default(DAE_ENHANCED_ENABLE);
    setting_comval->dae_enhanced_peak_ratio = (uint8) com_get_config_default(DAE_ENHANCED_PEAK_RATIO);
    setting_comval->dae_weaken_enable   = (uint8) com_get_config_default(DAE_WEAKEN_ENABLE);
    {
        dew_config_t tmp_dew_cfg;
        dew_band_t *p_dew_band;

        p_dew_band = sys_malloc(sizeof(dew_band_t)*MAX_DEW_SEG);
        if (p_dew_band == NULL)
        {
            PRINT_ERR("dew bands malloc fail!!");
            while (1)
            {
                ; //nothing for QAC
            }
        }

        for (i = 0; i < MAX_DEW_SEG; i++)
        {
            com_get_config_struct(DAE_ENHANCED_BAND0_SETTING + (uint16)i, (uint8 *) &tmp_dew_cfg,       \
                sizeof(dew_config_t));
            libc_memcpy(p_dew_band + i, &(tmp_dew_cfg.dew_band_setting), sizeof(dew_band_t));
        }

        sys_vm_write(p_dew_band, VM_DEW_PARAM_BASE, sizeof(dew_band_t)*MAX_DEW_SEG);

        for (i = 0; i < MAX_DEW_SEG; i++)
        {
            com_get_config_struct(DAE_WEAKEN_BAND0_SETTING + (uint16)i, (uint8 *) &tmp_dew_cfg,         \
                sizeof(dew_config_t));
            libc_memcpy(p_dew_band + i, &(tmp_dew_cfg.dew_band_setting), sizeof(dew_band_t));
        }

        sys_vm_write(p_dew_band, VM_DEW_PARAM_BASE + 0x10000, sizeof(dew_band_t)*MAX_DEW_SEG);

        sys_free(p_dew_band);
        p_dew_band = NULL;
    }

    p_dae_cfg->vbass_enable            = (uint8) com_get_config_default(DAE_VIRTUAL_BASS_ENABLE);
    p_dae_cfg->vbass_cut_freq          =         com_get_config_default(DAE_VIRTUAL_BASS_CUTOFF_FREQ);
    p_dae_cfg->vbass_ratio             = (int16) com_get_config_default(DAE_VIRTUAL_BASS_RATIO);
    p_dae_cfg->original_bass_ratio     = (int8)  com_get_config_default(DAE_ORIGINAL_BASS_RATIO);
    p_dae_cfg->vbass_type              = (int8)  com_get_config_default(DAE_VIRTUAL_BASS_TYPE);
    
    p_dae_cfg->vsurround_enable        = (uint8) com_get_config_default(DAE_VIRTUAL_SURROUND_ENABLE);
    p_dae_cfg->vsurround_angle         = (uint8) com_get_config_default(DAE_VIRTUAL_SURROUND_ANGLE);
    p_dae_cfg->vsurround_ratio         = (int8)  com_get_config_default(DAE_VIRTUAL_SURROUND_RATIO);

    p_dae_cfg->treble_enhance_enable   = (uint8) com_get_config_default(DAE_TREBLE_ENHANCE_ENABLE);
    p_dae_cfg->treble_enhance_cut_freq =         com_get_config_default(DAE_TREBLE_ENHANCE_CUTOFF_FREQ);
    p_dae_cfg->treble_enhance_ratio    = (int16) com_get_config_default(DAE_TREBLE_ENHANCE_RATIO);

    p_dae_cfg->mdrc_enable             = (uint8) com_get_config_default(DAE_MDRC_ENABLE);
    p_dae_cfg->limiter_enable          = (uint8) com_get_config_default(DAE_LIMITER_ENABLE);
    {
        mdrc_config_t tmp_limiter_config;

        com_get_config_struct(DAE_LIMITER_SETTING, (uint8 *) &tmp_limiter_config, sizeof(tmp_limiter_config));

        p_dae_cfg->limiter_threshold = tmp_limiter_config.mdrc_data.threshold;
        p_dae_cfg->limiter_attack_time  = (uint16)tmp_limiter_config.mdrc_data.attack_time;
        p_dae_cfg->limiter_release_time = (uint16)tmp_limiter_config.mdrc_data.release_time;
    }

}

//ÿ�ο���������ĳЩDAE���������
void com_reset_dae_config(comval_t *setting_comval)
{
    dae_config_t *p_dae_cfg;

    //DAE�ر�Ƕ�ײ㼶
    setting_comval->dae_off_nest = 0;
    setting_comval->signal_variable_mode = 0;
    setting_comval->signal_energy_used = 0;
    setting_comval->aux_flag = FALSE;

    p_dae_cfg = &(setting_comval->dae_cfg);

    p_dae_cfg->audiopp_type = (uint8) com_get_config_default(DAE_DEFAULT_TYPE);
 
    p_dae_cfg->p_mdrc_band_standard_mode = (dae_mdrc_band_standard_mode_t*)sys_malloc           \
        (sizeof(dae_mdrc_band_standard_mode_t)*MAX_MDRC_SEG_STANDARD);

    if (NULL == p_dae_cfg->p_mdrc_band_standard_mode)
    {
        PRINT_ERR("set mdrc_band_standard_mode malloc fail!!");
    }
   
    //Ĭ��ʹ��DAE
    p_dae_cfg->enable = TRUE;
    
#ifdef WAVES_ASET_TOOLS    
    p_dae_cfg->bypass = 1;   //ʹ��WAVES pc tools��actions dae bypassģʽ 
#else
    p_dae_cfg->bypass = (uint8) com_get_config_default(DAE_BYPASS_ENABLE);
#endif

#if (SUPPORT_MULTI_FREQ_MULTI_BAND == 1)
    dae_attributes_t*  dae_attributes = (dae_attributes_t*)sys_malloc(sizeof(dae_attributes_t));

    if (NULL == dae_attributes)
    {
        PRINT_ERR("set dae_attributes malloc fail!!");
    }

    p_dae_cfg->MultiFreqBandEnergy_enable = 0;
    {
        dae_attributes->num_band = 1;
        dae_attributes->duration_ms = 10;

        dae_attributes->f_low[0]  = 0;
        dae_attributes->f_high[0] = 200;//63
        dae_attributes->f_low[1]  = 63;
        dae_attributes->f_high[1] = 125;
        dae_attributes->f_low[2]  = 125;
        dae_attributes->f_high[2] = 250;
        dae_attributes->f_low[3]  = 250;
        dae_attributes->f_high[3] = 500;
        dae_attributes->f_low[4]  = 500;
        dae_attributes->f_high[4] = 1000;
        dae_attributes->f_low[5]  = 1000;
        dae_attributes->f_high[5] = 2000;
        dae_attributes->f_low[6]  = 2000;
        dae_attributes->f_high[6] = 4000;
        dae_attributes->f_low[7]  = 4000;
        dae_attributes->f_high[7] = 8000;
        dae_attributes->f_low[8]  = 8000;
        dae_attributes->f_high[8] = 16000;
        dae_attributes->f_low[9]  = 16000;
        dae_attributes->f_high[9] = 20000;
    }

    p_dae_cfg->FreqSpetrumDisplay_enable = 0;
    {
        dae_attributes->num_freq_point = 2;
        dae_attributes->freq_point[0] = 500;//31
        dae_attributes->freq_point[1] = 1000;//63
        dae_attributes->freq_point[2] = 125;
        dae_attributes->freq_point[3] = 250;
        dae_attributes->freq_point[4] = 500;
        dae_attributes->freq_point[5] = 800;
        dae_attributes->freq_point[6] = 1000;
        dae_attributes->freq_point[7] = 1500;
        dae_attributes->freq_point[8] = 2000;
        dae_attributes->freq_point[9] = 4000;
        dae_attributes->freq_point[10] = 8000;
        dae_attributes->freq_point[11] = 16000;
    }  

    p_dae_cfg->dae_attributes = dae_attributes;
#endif

#if (SPEAKER_HEADPHONE_FIX == 0)
    //�������ϵ���Ҫ����DAE
    if (get_headphone_state() == HEADPHONE_STATE_IN)
    {
        com_set_dae_onoff(FALSE);
    }
#endif

    com_load_mdrc_config(p_dae_cfg, TRUE, TRUE);
}

//�ýӿڽ�����CONFIG����ʱ����һ��
//ע�������ʼ����DAE���ò������ǲ���ȫ�ģ�����Ҫ������������֮��Ż�������Ч
void com_init_dae_config(comval_t *setting_comval)
{
    dae_config_t *p_dae_cfg;

    p_dae_cfg = sys_malloc(sizeof(dae_config_t));
    if (p_dae_cfg == NULL)
    {
        PRINT_ERR("dae_cfg malloc fail!!");
        while (1)
        {
            ; //nothing for QAC
        }
    }

    libc_memcpy(p_dae_cfg, &(setting_comval->dae_cfg), sizeof(dae_config_t));

    if (sys_shm_creat(SHARE_MEM_ID_DAECFG, p_dae_cfg, sizeof(dae_config_t)) == -1)
    {
        PRINT_ERR("dae_cfg shm create fail!!");
        while (1)
        {
            ; //nothing for QAC
        }
    }

    setting_comval->dae_inited_flag = TRUE;
}

//����mdrc����
void com_inner_load_mdrc_param(dae_config_t *p_dae_cfg, bool aux_flag)
{
    uint16 config_id;
    uint8 i;

    load_standard_mode_mdrc_param(p_dae_cfg,aux_flag);
        
    if (p_dae_cfg->mdrc_enable == 0)
    {
        return ;
    }

    sys_comval->dae_mdrc_peak_gain  = (uint8) com_get_config_default(DAE_PRECUT_RATIO);
    if (aux_flag == TRUE)
    {
        config_id = DAE_MDRC_AUX_BANK0_SETTING;
        p_dae_cfg->limiter_threshold_diff = (int16) com_get_config_default(DAE_MDRC_AUX_LIMITER_TH_DIFF);
        sys_comval->mdrc_vol_adjust = (int8) (int16) com_get_config_default(DAE_MDRC_AUX_VOLUME_ADJUST);
        p_dae_cfg->post_precut_ratio = (int8) (int16) com_get_config_default(DAE_MDRC_AUX_SIGNAL_ADJUST);
    }
    else
    {
        config_id = DAE_MDRC_BANK0_SETTING;
        p_dae_cfg->limiter_threshold_diff = (int16) com_get_config_default(DAE_MDRC_LIMITER_TH_DIFF);
        sys_comval->mdrc_vol_adjust = (int8) (int16) com_get_config_default(DAE_MDRC_VOLUME_ADJUST);
        p_dae_cfg->post_precut_ratio = (int8) (int16) com_get_config_default(DAE_MDRC_SIGNAL_ADJUST);
    }

    p_dae_cfg->mdrc_threshold_base_max = -600;
    for (i = 0; i < MAX_MDRC_SEG; i++)
    {
        mdrc_config_t tmp_mdrc_config;

        com_get_config_struct((uint16) (config_id + i), (uint8 *) &tmp_mdrc_config, \
                sizeof(tmp_mdrc_config));

        p_dae_cfg->mdrc_bands[i].threshold = tmp_mdrc_config.mdrc_data.threshold;
        p_dae_cfg->mdrc_bands[i].attack_time = tmp_mdrc_config.mdrc_data.attack_time;
        p_dae_cfg->mdrc_bands[i].release_time = tmp_mdrc_config.mdrc_data.release_time;
        p_dae_cfg->mdrc_bands[i].reserve = 0;

        if ((p_dae_cfg->limiter_enable == 1)
                && (p_dae_cfg->mdrc_bands[i].threshold > (0 - p_dae_cfg->limiter_threshold_diff)))
        {
            p_dae_cfg->mdrc_bands[i].threshold = (0 - p_dae_cfg->limiter_threshold_diff);
        }

        p_dae_cfg->mdrc_threshold_base[i] = p_dae_cfg->mdrc_bands[i].threshold;
        if (p_dae_cfg->mdrc_threshold_base[i] > p_dae_cfg->mdrc_threshold_base_max)
        {
            p_dae_cfg->mdrc_threshold_base_max = p_dae_cfg->mdrc_threshold_base[i];
        }

        if (i < (MAX_MDRC_SEG-1))
        {
            p_dae_cfg->mdrc_crossover_freq[i] = (uint16)tmp_mdrc_config.mdrc_data.crossover_freq;
        }
    }
}

void __section__(".text.bank")load_standard_mode_mdrc_param(dae_config_t *p_dae_cfg, bool aux_flag)
{
    uint16 config_id;
    mdrc_extend_para_temp_standard_mode_t compressor_temp;
    mdrc_para_temp_standard_mode_t        mdrc_temp;
    mdrc_peak_detect_temp_standard_mode_t  mdrc_peak_detect_temp;
    uint8 i = 0;
    int16 mdrc_threshold_max = -600;
     
    if (aux_flag == TRUE)
    {
        config_id = DAE_MDRC_AUX_EXPAND_BANK1_SETTING_STANDARD_MODE;

        com_get_config_struct((uint16) (config_id), (uint8 *)&compressor_temp ,             \
            sizeof(mdrc_extend_para_temp_standard_mode_t));
        sys_comval->mdrc_vol_adjust_standard_mode = (int8)compressor_temp.vol_adjust;
        libc_memcpy(&(p_dae_cfg->mdrc_extend_para_standard_mode), &compressor_temp.mdrc,    \
            sizeof(mdrc_extend_para_standard_mode_t));

        config_id = DAE_MDRC_AUX_BANK0_SETTING_STANDARD_MODE;
     
        for (i = 0; i < MAX_MDRC_SEG_STANDARD; i++)
        {
            com_get_config_struct((uint16) (config_id + i), (uint8 *)&mdrc_temp , sizeof(mdrc_para_temp_standard_mode_t));
            libc_memcpy((p_dae_cfg->p_mdrc_band_standard_mode) + i, &(mdrc_temp.mdrc_band), sizeof(dae_mdrc_band_standard_mode_t));
        }      
             
        //MDRC��ֵ�����ز���
        config_id = MDRC_PEAK_AUX_DETECTCOMPENSATION;
        com_get_config_struct((uint16) (MDRC_PEAK_AUX_DETECTCOMPENSATION), (uint8 *)&mdrc_peak_detect_temp , \
            sizeof(mdrc_peak_detect_temp_standard_mode_t));
        libc_memcpy(&(p_dae_cfg->mdrc_peak_standard_mode), &mdrc_peak_detect_temp.mdrc_peak_detect,           \
            sizeof(mdrc_peak_detect_standard_mode_t));
    }
    else
    {
        config_id = DAE_MDRC_EXPAND_BANK1_SETTING_STANDARD_MODE;

        com_get_config_struct((uint16) (config_id), (uint8 *)&compressor_temp ,             \
            sizeof(mdrc_extend_para_temp_standard_mode_t));
        sys_comval->mdrc_vol_adjust_standard_mode = (int8)compressor_temp.vol_adjust;
        libc_memcpy(&(p_dae_cfg->mdrc_extend_para_standard_mode), &compressor_temp.mdrc,    \
            sizeof(mdrc_extend_para_standard_mode_t));

        config_id = DAE_MDRC_BANK0_SETTING_STANDARD_MODE;
   
        for (i = 0; i < MAX_MDRC_SEG_STANDARD; i++)
        {
           com_get_config_struct((uint16) (config_id + i), (uint8 *)&mdrc_temp , sizeof(mdrc_para_temp_standard_mode_t));
           libc_memcpy((p_dae_cfg->p_mdrc_band_standard_mode) + i, &(mdrc_temp.mdrc_band), sizeof(dae_mdrc_band_standard_mode_t));
        }
                    
        //MDRC��ֵ�����ز���
        config_id = MDRC_PEAK_DETECTCOMPENSATION;
        com_get_config_struct((uint16) (MDRC_PEAK_DETECTCOMPENSATION), (uint8 *)&mdrc_peak_detect_temp , \
            sizeof(mdrc_peak_detect_temp_standard_mode_t));
        libc_memcpy(&(p_dae_cfg->mdrc_peak_standard_mode), &mdrc_peak_detect_temp.mdrc_peak_detect, \
            sizeof(mdrc_peak_detect_standard_mode_t));
    }       
}