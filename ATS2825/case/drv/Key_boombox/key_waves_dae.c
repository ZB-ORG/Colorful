/*******************************************************************************
 *                         
 * WAVES PC工具和算法数据接口代码
 *
 *******************************************************************************/
#include <psp_includes.h>

static void write_waves_dae_para_vm(void)
{
    uint8 i = 0;
    uint8 block_num = 0;
    waves_dae_para_t* waves_dae_para = (waves_dae_para_t*)WAVES_PC_TOOLS_DATA_START_ADDR;

    block_num = (WAVES_PC_TOOLS_DATA_END_ADDR - WAVES_PC_TOOLS_DATA_START_ADDR + 2)/WAVES_PC_TOOLS_DATA_LENGTH + 1;

    for (i = 0; i < block_num; i++)
    {
        sys_vm_write(waves_dae_para + i, (VM_WAVES_AL_DATA1 + (i<<16)), WAVES_PC_TOOLS_DATA_LENGTH);
    }   
}


static void read_waves_dae_para_vm(void)
{
    uint8 i = 0;
    uint8 block_num = 0;
    waves_dae_para_t* waves_dae_para = (waves_dae_para_t*)WAVES_PC_TOOLS_DATA_START_ADDR;

    block_num = (WAVES_PC_TOOLS_DATA_END_ADDR - WAVES_PC_TOOLS_DATA_START_ADDR + 2)/WAVES_PC_TOOLS_DATA_LENGTH + 1;

    for (i = 0; i < block_num; i++)
    {
        sys_vm_read(waves_dae_para + i, (VM_WAVES_AL_DATA1 + (i<<16)), WAVES_PC_TOOLS_DATA_LENGTH);
    }   
}


int32 key_inner_set_waves_effect_param(uint32 set_type, void *param_ptr, void *null3)
{
     coeff_t *p_coeff = (coeff_t*)WAVES_ASET_COEFF_ADDR;
     coeff_property_t* p_coeff_property;

     uint32 cur_package_start_addr = 0;
     uint32 cur_package_length = 0;
        
     switch (set_type)
     {
        case SET_WAVES_COEFF_PROPERTY:
        p_coeff_property = (coeff_property_t*)param_ptr; 
        
        p_coeff->parameter_address = p_coeff_property->memory_addr;    
        p_coeff->parameter_length = p_coeff_property->length;
             
        break;
        
        case SET_WAVES_EFFECT_PARAM:
            
        cur_package_start_addr = p_coeff->parameter_address; 
        cur_package_length = p_coeff->parameter_length;

        libc_memcpy((void*)(cur_package_start_addr - 0x2d800 + WAVES_ASET_SHARE_MEM_START_ADDR),(uint8*)param_ptr,cur_package_length);

        p_coeff->DAE_change_flag = 1;      
        break;

        case WAVES_DAE_WRITE_VRAM:
           
        write_waves_dae_para_vm();
        
        break;

        case WAVES_DAE_READ_VRAM:
           
        read_waves_dae_para_vm();
        
        break;
        
        default:
        break;
     }
    return 0;
}


