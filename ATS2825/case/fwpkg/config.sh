#!/bin/bash

# This script modify config.txt, fwimage.cfg and source files
# config.txt --> used for configure function support or not
# fwimage.cfg--> used for configure function support or not
# source file--> open or close some C source files
# 
# 2016-06-12	-	wuyufan

# 该脚本文件用于配置源文件，包括config.txt,fwimage_xx.cfg文件等，
# 也会修改部分源文件的宏定义来打开或关闭某些功能
# 当板型或功能有所变更时，必须先调用config.sh配置源文件，然后再进行程序的编译
# 该脚本需要传递三个参数，第一个参数，支持的板型(EVB, DVB_ATS2825, DVB_ATS2823, EVB_ATS2823)
# 第二个参数，是否支持OTA
# 第三个参数，是否支持TWS
# 目前OTA和TWS功能不能同时打开

echo "config source file"

FW_NAME_EVB=US282A_BTBOX_EVB
FW_NAME_DVB_2825=US282A_BTBOX_DVB_ATS2825
FW_NAME_DVB_2823=US282A_BTBOX_DVB_ATS2823
FW_NAME_EVB_2823=US282A_BTBOX_EVB_ATS2823

if [ $# != 3 ]
then
    echo "Enter Board type[0/1/2]:"
    echo "0: US282A_BTBOX_EVB"
    echo "1: US282A_BTBOX_DVB_ATS2825"
    echo "2: US282A_BTBOX_DVB_ATS2823"
    echo "3: US282A_BTBOX_EVB_ATS2823"
        
    read -n1 BoardNum
      
    case $BoardNum in
    0)
          BoardType=$FW_NAME_EVB
          echo "right Board type:$BoardType";;
    1)
          BoardType=$FW_NAME_DVB_2825
          echo "right Board type:$BoardType";;
    2)
          BoardType=$FW_NAME_DVB_2823  
          echo "right Board type:$BoardType";;
    3)
          BoardType=$FW_NAME_EVB_2823  
          echo "right Board type:$BoardType";;          
    *)
          echo "error Board type:$BoardType"
          exit 1;;
    esac
          
  	read -n1 -p "Support OTA or Not:[y/n]" SupportOTA
  	
  	echo -e '\n'

	case $SupportOTA in
	Y|y)
	      SupportOTA=1
	      echo "SDK support OTA";;
	N|n)
	      SupportOTA=0
	      echo "SDK Not support OTA";; 
	*)
	      echo "error choice"
	      exit 1;;    
	esac
	
	read -n1 -p "Support TWS or Not:[y/n]" SupportTWS
	
	echo -e '\n'
	case $SupportTWS in
	Y|y)
	      if [ $SupportOTA == 1 ]
	      then
	          echo "SDK Not support OTA and TWS"
	          exit 1
	      else
	          SupportTWS=1
	          echo "SDK support TWS"
	      fi;;  	
	N|n)
	      SupportTWS=0
	      echo "SDK Not support TWS";; 
	*)
	      echo "error choice"
	      exit 1;;
	esac		
else
    BoardType=$1
    SupportOTA=$2
    SupportTWS=$3	
fi

    config_txt_name=$BoardType.txt
    config_cfg_name="fwimage_"$BoardType".cfg"  
    
if [ ! -f "./config_txt/$config_txt_name" ]
then
    echo "ConfigError: ./config_txt/$config_txt_name not exist"
    exit 1;
fi   

if [ ! -f "./config_txt/$config_cfg_name" ]
then
    echo "ConfigError: ./config_txt/$config_cfg_name not exist"
    exit 1;
fi  
    
if [ $SupportOTA == 1 ]
then   	
	sed '0,/INF_MAKE_FW_MODE.*;/s/INF_MAKE_FW_MODE.*;/INF_MAKE_FW_MODE = 1;/' ./config_txt/$config_cfg_name > tmpfile
	mv tmpfile ./config_txt/$config_cfg_name  
else	
	sed '0,/INF_MAKE_FW_MODE.*;/s/INF_MAKE_FW_MODE.*;/INF_MAKE_FW_MODE = 0;/' ./config_txt/$config_cfg_name > tmpfile
	mv tmpfile ./config_txt/$config_cfg_name  	
fi  

if [ $BoardType = "$FW_NAME_DVB_2823" ]
then
	sed '0,/BTMANAGER_ENABLE_SPP.*;/s/BTMANAGER_ENABLE_SPP.*;/BTMANAGER_ENABLE_SPP                 = 0[0,1];/' ./config_txt/$config_txt_name > tmpfile
	mv tmpfile ./config_txt/$config_txt_name 

	sed '0,/BTMANAGER_ENABLE_BLE.*;/s/BTMANAGER_ENABLE_BLE.*;/BTMANAGER_ENABLE_BLE                 = 0[0,1];/' ./config_txt/$config_txt_name > tmpfile
	mv tmpfile ./config_txt/$config_txt_name 
else			
	sed '0,/BTMANAGER_ENABLE_SPP.*;/s/BTMANAGER_ENABLE_SPP.*;/BTMANAGER_ENABLE_SPP                 = 1[0,1];/' ./config_txt/$config_txt_name > tmpfile
	mv tmpfile ./config_txt/$config_txt_name 

	sed '0,/BTMANAGER_ENABLE_BLE.*;/s/BTMANAGER_ENABLE_BLE.*;/BTMANAGER_ENABLE_BLE                 = 1[0,1];/' ./config_txt/$config_txt_name > tmpfile
	mv tmpfile ./config_txt/$config_txt_name 	 
fi 

if [ $SupportTWS == 1 ]
then   
	sed '0,/BTMANAGER_SUPPORT_DEVICE_NUM.*;/s/BTMANAGER_SUPPORT_DEVICE_NUM.*;/BTMANAGER_SUPPORT_DEVICE_NUM         = 2[1~2,1];/' ./config_txt/$config_txt_name > tmpfile
	mv tmpfile ./config_txt/$config_txt_name   

	sed '0,/^\/\/\#define ENABLE_TRUE_WIRELESS_STEREO.*/s/^\/\/\#define ENABLE_TRUE_WIRELESS_STEREO.*/\#define ENABLE_TRUE_WIRELESS_STEREO/' ../inc/btstack_common.h > tmpfile
	mv tmpfile ../inc/btstack_common.h
	
	sed '0,/BTMANAGER_ENABLE_SPP.*;/s/BTMANAGER_ENABLE_SPP.*;/BTMANAGER_ENABLE_SPP                 = 0[0,1];/' ./config_txt/$config_txt_name > tmpfile
	mv tmpfile ./config_txt/$config_txt_name 

	sed '0,/BTMANAGER_ENABLE_BLE.*;/s/BTMANAGER_ENABLE_BLE.*;/BTMANAGER_ENABLE_BLE                 = 0[0,1];/' ./config_txt/$config_txt_name > tmpfile
	mv tmpfile ./config_txt/$config_txt_name 	
else
	sed '0,/BTMANAGER_SUPPORT_DEVICE_NUM.*;/s/BTMANAGER_SUPPORT_DEVICE_NUM.*;/BTMANAGER_SUPPORT_DEVICE_NUM         = 1[1~2,1];/' ./config_txt/$config_txt_name > tmpfile
	mv tmpfile ./config_txt/$config_txt_name  

	sed '0,/^\#define ENABLE_TRUE_WIRELESS_STEREO.*/s/^\#define ENABLE_TRUE_WIRELESS_STEREO.*/\/\/\#define ENABLE_TRUE_WIRELESS_STEREO/' ../inc/btstack_common.h > tmpfile
	mv tmpfile ../inc/btstack_common.h  	
fi	
	
#根据输入参数判断要修改的头文件位置
if [ $BoardType = "$FW_NAME_EVB" ]
then 	
	#修改case_independent.h头文件BOARD类型为CASE_BOARD_EVB
	sed '0,/CASE_BOARD_TYPE.*CASE_BOARD.*/s/CASE_BOARD_TYPE.*CASE_BOARD.*/CASE_BOARD_TYPE         CASE_BOARD_EVB/' ../inc/case_independent.h > tmpfile
	mv tmpfile ../inc/case_independent.h    
elif [ $BoardType = "$FW_NAME_DVB_2825" ]
then 
	#修改case_independent.h头文件BOARD类型为CASE_BOARD_DVB_ATS2825
	sed '0,/CASE_BOARD_TYPE.*CASE_BOARD.*/s/CASE_BOARD_TYPE.*CASE_BOARD.*/CASE_BOARD_TYPE         CASE_BOARD_DVB_ATS2825/' ../inc/case_independent.h > tmpfile
	mv tmpfile ../inc/case_independent.h   	
elif [ $BoardType = "$FW_NAME_DVB_2823" ]
then 	
	#修改case_independent.h头文件BOARD类型为CASE_BOARD_DVB_ATS2823
	sed '0,/CASE_BOARD_TYPE.*CASE_BOARD.*/s/CASE_BOARD_TYPE.*CASE_BOARD.*/CASE_BOARD_TYPE         CASE_BOARD_DVB_ATS2823/' ../inc/case_independent.h > tmpfile
	mv tmpfile ../inc/case_independent.h  			
elif [ $BoardType = "$FW_NAME_EVB_2823" ]
then 	
	#修改case_independent.h头文件BOARD类型为CASE_BOARD_EVB_ATS2823
	sed '0,/CASE_BOARD_TYPE.*CASE_BOARD.*/s/CASE_BOARD_TYPE.*CASE_BOARD.*/CASE_BOARD_TYPE         CASE_BOARD_ATS2823/' ../inc/case_independent.h > tmpfile
	mv tmpfile ../inc/case_independent.h  	
else
	echo "unknown case type!!!"
	echo "Support Type:US282A_BTBOX_EVB/US282A_BTBOX_DVB_ATS2825/US282A_BTBOX_DVB_ATS2823/US282A_BTBOX_EVB_ATS2823"
	exit 1
fi

#在编译之前生成config.h，保证更新config.txt,程序可正常编译
echo "*************************Build config.bin******************************"
../tools/Gen_config/Genheader.exe     ./config_txt/$config_txt_name      ../inc/config_id.h

../tools/Gen_config/Genbin.exe          ./config_txt/$config_txt_name         ./bin/config.bin

#比较不同板型生成的config_id.h是否一致，如果不一致，说明配置项有漏加的情况，需要做同步
#如果方案只有一个板型，可以去除以下部分语句
#echo "*************************Compare config.bin******************************"
#../tools/Gen_config/Genheader.exe     ./config_txt/US282A_BTBOX_EVB.txt   tmpfile   
#
#diff tmpfile ../inc/config_id.h
#if [ $? != 0 ]
#then
#  echo -e "config.bin is different!"
#	sleep 3s;
#	rm -f tmpfile
#	exit 1		
#else
#  echo -e "config.bin is same"
#  rm -f tmpfile
#fi	
    


 
