# **********************************************
# Name of application
# **********************************************

IMAGENAME = drv_fm
# **********************************************
# Type of Processor	& Board
# **********************************************
Processor =	mips
# **********************************************
# Directories
# **********************************************

#把公共路径加载进来
include	./../../cfg/common_path
#把公共定义makefile加载进来
include	./../../cfg/rules.mk
SRC = $(CASE)/drv
SRCDIR_16 = $(SRC)/fm_qn8035 $(SRC)/fm_common $(SRC)/fm_qn8035/QN
SRCDIR_32 =

#目标文件送往的文件夹路径,开发人员可能需修改处(modify-3)
OBJECT_BIN_PATH = $(CASE)/fwpkg/drv
OBJECT_ROM_BIN_PATH = $(PSP_REL)/bin_original
# **********************************************


#工具名称
DRV_BUILDER = $(PSP_REL)/tools/drv_builder.exe
AP_BUILDER = $(PSP_REL)/tools/ap_builder.exe   
#存放结果文件的目录，这里的设置为当前目录，即生成的结果文件放在当前目录
LINKDIR	  =	.
#指定依赖过程的文件的搜索路径，把源文件的路径写上即可
VPATH	=	$(SRCDIR_16) $(SRCDIR_32)

# **********************************************
# Image	file names and map file
# **********************************************
#指定结果文件的名称
IMAGE_ELF =	$(IMAGENAME).exe
IMAGE_LST =	$(IMAGENAME).lst
IMAGE_REC =	$(IMAGENAME).rec
IMAGE_BIN =	$(IMAGENAME).sys
MAP		  =	$(IMAGENAME).map
TARGET_DRV = $(IMAGENAME).drv

# **********************************************
# Compiler and linker options
# **********************************************

#指定程序中引用的头文件的搜索路径，一般源码include时只写上头文件的名称，没有路径信息。这时就根据这个选项去搜索该头文件
INCLUDE	  =	 -I$(PSP_REL)/include/ucos -I$(PSP_REL)/include -I$(CASE)/inc -I$(CASE)/drv/fm_common/inc -I$(SRC)/fm_qn8035  -I$(SRC)/fm_qn8035/QN 

#指定链接时搜索的路径
LINCLUDE = -L$(PSP_REL)/include -L$(PSPLIBDIR) -L$(SRC)/rom_TWI_lib


#指定自定义链接脚本的名称,开发人员需修改处(modify-4)
LD_SCRIPT = fm_driver.xn

#链接命令行选项
LD_OPTS = $(LINCLUDE) -G 0 -T $(LD_SCRIPT) -o $(IMAGE_ELF) -Map $(MAP) $(OFORMAT) 
LD_OPTS_1   = $(LINCLUDE) -G0 -T $(LD_SCRIPT) $(OFORMAT)
LD_OPTS_2   = -o $(IMAGE_ELF) -Map $(MAP)


# **********************************************
# Files	to be compiled
# **********************************************

#获得.c后缀源码
SRC_C_16  = $(foreach dir,	$(SRCDIR_16), $(wildcard $(dir)/*.c))
#转换为.o格式文件名称，不带路径信息
OBJ_C_16  = $(notdir $(patsubst %.c, %.o, $(SRC_C_16)))

#获得.c后缀源码
SRC_C_16_O2  = $(foreach dir,	$(SRCDIR_16_O2), $(wildcard $(dir)/*.c))
#转换为.o格式文件名称，不带路径信息
OBJ_C_16_O2  = $(notdir $(patsubst %.c, %.o, $(SRC_C_16_O2)))

#获得.c后缀源码
SRC_C_32  = $(foreach dir,	$(SRCDIR_32), $(wildcard $(dir)/*.c))
#转换为.o格式文件名称，不带路径信息
OBJ_C_32  = $(notdir $(patsubst %.c, %.o, $(SRC_C_32)))

#获得.S后缀源码
SRC_S_16  = $(foreach dir,	$(SRCDIR_16), $(wildcard $(dir)/*.S))
#转换为.o格式文件名称，不带路径信息
OBJ_S_16  = $(notdir $(patsubst %.S, %.o, $(SRC_S_16)))

#获得.S后缀源码
SRC_S_32  = $(foreach dir,	$(SRCDIR_32), $(wildcard $(dir)/*.S))
#转换为.o格式文件名称，不带路径信息
OBJ_S_32  = $(notdir $(patsubst %.S, %.o, $(SRC_S_32)))

#获得所有的.o文件名称
OBJ = $(OBJ_C_16) $(OBJ_C_16_O2) $(OBJ_S_16) $(OBJ_C_32) $(OBJ_S_32)


# **********************************************
# Rules
# **********************************************
#make目标
.PHONY : all

#dump出lst文件
all : $(IMAGE_ELF)
#	$(OBJDUMP) -D $(IMAGE_ELF) > $(IMAGE_LST)
	$(DRV_BUILDER) $(IMAGE_ELF) $(TARGET_DRV)
	cp -f $(TARGET_DRV) $(OBJECT_BIN_PATH)	
	
#链接过程
$(IMAGE_ELF) : $(OBJ)
	$(LD) $(LD_OPTS_1) $(OBJ) $(LD_OPTS_2)

#编译过程	
$(OBJ_C_16) : %.o : %.c
	$(CC) $(CC_OPTS_O0_16) -o	$@ $<
	@echo
	
$(OBJ_S_16) : %.o : %.S
	$(CC) $(CC_OPTS_O0_A_16) -o $@ $<
	@echo

$(OBJ_C_32) : %.o : %.c
	$(CC) $(CC_OPTS_O0) -o	$@ $<
	@echo
	
$(OBJ_S_32) : %.o : %.S
	$(CC) $(CC_OPTS_O0_A) -o $@ $<
	@echo
	
#删除结果文件
.PHONY : clean

clean :
	rm -f $(LINKDIR)/*.o
	rm -f $(LINKDIR)/$(IMAGENAME).*

	