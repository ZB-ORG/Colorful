/********************************************************************************
 *                              GL5116
 *                            Module: music_player
 *                 Copyright(c) 2003-2011 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 *      History:
 *      <author>            <time>           <version >             <des>
 *     zhuangyongkang    2016-05-26 10:00       1.0             Standard Sound Models
 ********************************************************************************/

#ifndef _DAE_PARA_STANDARD_MODE_INFO_DEF_H
#define _DAE_PARA_STANDARD_MODE_INFO_DEF_H

#include <typeext.h>
#include <attr.h>
#include <dae_info_def.h>

//NOTE:标准音效模型
#define EQ_NUM_BANDS_STANDARD_MODE             EQ_NUM_BANDS
#define MDRC_NUM_BANDS_STANDARD_MODE            3

struct eq_band_setting_standard_mode    
{
    short cutoff;                       //中心频率，20:1:24000Hz
    short q;                            //0.3:0.1:3，传值扩大10倍，即范围-3:1:30
    short gain;                         //-20:0.1:12dB，传值扩大10倍，即范围-200:1:120
    short type;                         ///*滤波器类型：0是无效，1是常规EQ滤波器，2是高通滤波器，其它值为reserve*/
};

struct mdrc_band_setting_standard_mode
{
    short threshold1;
    short compress_ratio1;
	short threshold2;
    short compress_ratio2;
	short tav_time;
    short attack_time;
    short release_time;
	short reserve;//确保4字节对齐
};

typedef struct
{

    int sample_rate;            //采样率，支持44100,48000Hz两种
    int channels;               //目前都是按双通道处理
    int block_size;             //帧长，目前为128点长度
	
	int fade_in_time_ms;        //淡入长度，[50 100 200 300 400 500]ms
    int fade_out_time_ms;       //淡出长度，[50 100 200 300 400 500]ms
    int mute_time_ms;           //静音长度，长度没有限制，单位ms

    int fade_in_flag;           //淡入标志位，置1表示下一帧开始淡入
    int fade_out_flag;          //淡出标志位，置1表示下一帧开始淡出
    int mute_flag;              //静音标志位，置1表示下一帧开始静音
	
    int DAE_init_flag;          //初始化标识位
    int DAE_change_flag;        //DAE全部参数改变标识位
    short  output_channel_config;//1:左右声道交换 2: 单端耳机应用(L=R=原始的(L/2 + R/2)), 0 or 其它值, 跟原始的左右声道一致；该功能与音效无关，不受 BYPASS 影响
    short  bypass;               //bypass,1表示直通，0表示DAE处理
    int precut;                 //预衰减，初始化为0；precut 不受 BYPASS 影响
	/***********************************/

//确保4字节对齐
    short Vbass_enable;         //Vbass开关
    short Surround_enable;      //Surround开关
    short TrebleEnhance_enable; //Treble开关
    short Peq_enable;           //Peq开关
    short SpeakerCompensation_enable;//SpeakerCompensation开关
	short Compressor_enable;
    short MultibandDRC_enable;  //MDRC开关
    short reserve_0;

//确保4字节对齐
    /***********************************/
    int Vbass_low_cut_off_frequency;//vbass低音频率，初始化建议80hz
    int Vbass_gain;             //vbass增益,单位为0.1dB，-120：10：120dB，初始化建议60dB
    int Vbass_type;             //虚拟低音的类型
    /***********************************/
    int Surround_angle;         //10:10:60°,双扬声器到人头的夹角，初始化为10°
    int Surround_gain;          //环绕声增益，-60:1:0dB，初始化为-3dB
    /***********************************/
    int Treble_frequency;       //高音增强开始频率，8000:1000:20000Hz，初始化为10000Hz
    int Treble_gain;            //高音增强幅度,单位为0.1dB，0:10:150dB，初始化为100dB
    /***********************************/
    //int MDRC_change_flag;     //MDRC单独的参数改变标识位，MDRC参数改变时置1，同时DAE_change_flag置1
    int crossover_freqency[MDRC_NUM_BANDS_STANDARD_MODE-1];
    short MDRC_compensation_peak_detect_attack_time;	//default: 0.5ms, step: 0.01ms, setting: 5*10
	short MDRC_compensation_peak_detect_release_time;	//default: 500ms, step: 1ms, setting: 500
	short MDRC_compensation_threshold;				//default: -100,step 0.1dB,setting: -10*10 = -100
	short MDRC_compensation_filter_Q;					//default: 1,step 0.1, setting:0.1*10 = 1, 0.1~0.9
    /***********************************/
    int Peq_change_flag;        //peq单独的参数改变标识位，peq参数改变时置1，同时DAE_change_flag置1
    struct eq_band_setting_standard_mode band_settings[EQ_NUM_BANDS_STANDARD_MODE]; //peq频点参数，当band_settings中四个参数中有改变时Peq_change_flag、DAE_change_flag同时置1
    short band_enable_type[EQ_NUM_BANDS_STANDARD_MODE]; //peq类型和enable选项，1表示Speaker PEQ，2表示Post PEQ，0表示不执行
    /***********************************/
    short SpeakerCompensationChangeFlag;
    short FilterOrder;
    short FilterQvalLeft;
    short FilterQvalRight;
    /***********************************/
    int makeup_gain;            //makeup gain 大音量下调试用，用来调整灵敏度，小音量下为0
	int makeup2;                //将信号补到0db    
    /***********************************/
    short energy_detect_enable; //信号能量检测使能
    short period;               //统计周期时长，以1ms为单位
    short period_count;         //有效周期数
    short preadjust_db;         //信号大小降低较多才启动预调机制，以0.1db为单位
    short preadjust_count;      //开始预调降低信号大小的周期数，然后每个周期降低相应比例
//signal_energy_inner必须放在energy前一个位置
    short signal_energy_inner;  //！！方案端去读该值！！ 切换信号检测时存放初始值
    /***********************************/
    short  energy;     //解码帧pcm平均值
    short  energy_max; //解码帧pcm绝对值最大值
//以上3个成员必须紧挨着
    short  max_vol_flag;
    short  reserve_2;//确保4字节对齐
    short  reserve_3;
    short  reserve_4;
    short  reserve_5;
    short  reserve_6;
    short  reserve_7;
    short  reserve_8;

    /***********************************/
	int Compressor_threshold1;
    int Compressor_ratio1;
	int Compressor_threshold2;
    int Compressor_ratio2;
	int Compressor_tav;
	int Compressor_attack_time;
	int Compressor_release_time;
    
    struct mdrc_band_setting_standard_mode mdrc_band_settings[MDRC_NUM_BANDS_STANDARD_MODE]; //MDRC参数
  
}DAE_para_info_standard_mode_t;


#endif //_DAE_PARA_H
