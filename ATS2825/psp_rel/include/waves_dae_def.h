#ifndef __WAVES_DAE_DEF__
#define __WAVES_DAE_DEF__
#include <psp_includes.h>

/*********WAVES PC工具和小机交互时所需要的临时缓冲数据空间*********/
//0x9fc3afe0-9fc3afff(32字节)地址存放系数表
#define WAVES_ASET_COEFF_ADDR                     (0x9fc3afe0)                        
//waves公司算法，需要占用DSP4k的空间 0x9fc3b000-0x9fc3bfff
#define WAVES_ASET_SHARE_MEM_START_ADDR           (0x9fc3b000)
#define WAVES_ASET_SHARE_MEM_END_ADDR             (0x9fc3bfff)
/******************************************************************/

/*********waves算法加载后运行数据访问的RAM空间*********************/
//地址:0x9fc388b8-0x9fc3a416(0x2c45c-0x2d20b)


/*****小机和WAVES PC工具交互的时候，其数据最终存放的起始地址是*****/
/*****0x9fc388b8，长度小于512字节，为避免工具调试时,切换歌曲音*****/
/*****效参数失效，从0x9fc388b8起始地址后的512字节应当备份**********/
#define WAVES_PC_TOOLS_DATA_START_ADDR            (0x9fc388b8)
#define WAVES_PC_TOOLS_DATA_END_ADDR              (0x9fc3a416) 

#define WAVES_PC_TOOLS_DATA_LENGTH                (512) 

#define VM_WAVES_AL_DATA1                         (0x00310000)
#define VM_WAVES_AL_DATA2                         (0x00320000) 
#define VM_WAVES_AL_DATA3                         (0x00330000) 
#define VM_WAVES_AL_DATA4                         (0x00340000) 
#define VM_WAVES_AL_DATA5                         (0x00350000) 
#define VM_WAVES_AL_DATA6                         (0x00360000) 
#define VM_WAVES_AL_DATA7                         (0x00370000) 
#define VM_WAVES_AL_DATA8                         (0x00380000) 
#define VM_WAVES_AL_DATA9                         (0x00390000) 
#define VM_WAVES_AL_DATA10                        (0x003a0000) 
#define VM_WAVES_AL_DATA11                        (0x003b0000) 
#define VM_WAVES_AL_DATA12                        (0x003c0000) 
#define VM_WAVES_AL_DATA13                        (0x003d0000) 
#define VM_WAVES_AL_DATA14                        (0x003e0000) 


typedef struct
{
    uint8  data[WAVES_PC_TOOLS_DATA_LENGTH];
}waves_dae_para_t;
#endif


typedef struct
{
    uint32  update_flag;            //1表示PC工具更新了系数表，需要传递到DSP LIB
    uint32  length;                 //此次要更新的系数表的长度
    uint32  memory_addr;            //将要更新内存的地址       
}coeff_property_t;

typedef struct
{
    int DAE_change_flag;
    int parameter_address;
    int parameter_length;   
    int reserve[5];       
}coeff_t;

typedef struct
{    
    coeff_t  coeff_table;                 
    uint8    data[248]; 
}waves_para_vram_t;


typedef enum
{
    /*!设置系数表*/
    SET_WAVES_COEFF_PROPERTY,
    /*!设置音效配置参数*/
    SET_WAVES_EFFECT_PARAM,
    /*!音效参数从内存写到VRAM*/
    WAVES_DAE_WRITE_VRAM,
    /*!音效参数从内存写到VRAM*/
    WAVES_DAE_READ_VRAM,
} set_waves_effect_type_e;



