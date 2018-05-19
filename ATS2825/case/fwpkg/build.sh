#!/bin/bash

# This script build these files:
# *.FW---> firmware 
# *.OTA--> OTA firmare
# *.HEX--> upgrade file for end user
# *.FWU--> upgrade file for factory product
# *.ATF--> autotest file for ATT
# 2016-06-12	-	wuyufan

# 该脚本文件用于生成最终量产的fw文件，OTA文件，HEX升级文件以及ATT工具所需的atf文件
# 如果通过makefile调用该脚本，则需要传递两个参数，支持的板型(EVB, DVB_ATS2825, DVB_ATS2823, EVB_ATS2823)，以及生成的固件名称(不包括后缀)
# 如果单独在命令行调用该脚本，则需要选择支持的板型，以及是否支持TWS,OTA等功能

echo "making fw"

FW_NAME_EVB=US282A_BTBOX_EVB
FW_NAME_DVB_2825=US282A_BTBOX_DVB_ATS2825
FW_NAME_DVB_2823=US282A_BTBOX_DVB_ATS2823
FW_NAME_EVB_2823=US282A_BTBOX_EVB_ATS2823

echo "case type $1"

if [ $# != 2 ]
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
	      FW_NAME=$BoardType"_OTA"
	      echo "SDK support OTA";;
	N|n)
	      SupportOTA=0
	      FW_NAME=$BoardType
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
    	      FW_NAME=$FW_NAME"_TWS"
    	      echo "SDK support TWS"
	      fi;;  
	N|n)
	      SupportTWS=0
	      echo "SDK Not support TWS";; 
	*)
	      SupportTWS=0
	      echo "error choice"
	      exit 1;;
	esac
else
    BoardType=$1
    FW_NAME=$2		
fi
#判断之前编译的结果是否正确,如果编译结果不正确，则不再做固件生成操作
grep "compile error" make.log 
if [ $? == 0 ]
then
    echo "make error-->exit"
    exit 1;
fi

#判断内存是否正确
grep "Error :section" make.log 
if [ $? == 0 ]
then
    echo "make error-->exit"
    exit 1;
fi

#字符串拼接，获取config.txt文件名和fwimage.cfg的文件名
config_txt_name=$BoardType".txt"

config_cfg_name="fwimage_"$BoardType".cfg"  

echo $config_txt_name

grep "INF_MAKE_FW_MODE .*1" ./config_txt/$config_cfg_name
if [ $? == 0 ]
then
    SupportOTA=1
else
    SupportOTA=0;
fi

grep "//#define ENABLE_TRUE_WIRELESS_STEREO" ../inc/btstack_common.h
if [ $? == 0 ]
then
    SupportTWS=0
else
    SupportTWS=1;
fi

echo "Support OTA: $SupportOTA" 
echo "Support TWS: $SupportTWS" 

if [ $SupportOTA == 1 ]
then
    config_cfg_name="fwimage_"$BoardType"_OTA.cfg"          
fi  

if [ -d "../ap/bluetooth_stack" ]
then
    if [ $SupportTWS == 1 ]
    then 
        echo "copy btstack_tws.ap" 
        cp -f ../ap/bluetooth_stack/btstack.ap ./ap/btstack_tws.ap        
    else
        echo "copy btstack_normal.ap" 
        cp -f ../ap/bluetooth_stack/btstack.ap ./ap/btstack_normal.ap
    fi
fi

if [ $SupportTWS == 1 ]
then  
    cp -f ./ap/btstack_tws.ap  ./ap/btstack.ap      
else
    cp -f ./ap/btstack_normal.ap ./ap/btstack.ap 
fi

cp -f ./config_txt/$config_txt_name ./bin/config.txt
cp -f ./config_txt/$config_cfg_name ./fwimage.cfg

../tools/Gen_config/Genbin.exe          ./config_txt/$config_txt_name         ./bin/config.bin

chmod -R 777 ./

echo "*************************Reduce AP Size******************************"
../../psp_rel/tools/ap_post_builder.exe ./fwimage.cfg config.ap


echo "*************************Build AFI *****************************"
if [ -f "../../psp_rel/bin_original/buildAFI.bat" ]
then
    ./buildAFI.bat
fi    
				
echo "*************************Build Firmware *****************************"
../../psp_rel/tools/maker_py/Maker.exe -c ./fwimage.cfg -o  ./$FW_NAME.FW -mf

#rm -f ./bin/config.txt
#rm -f ./bin/config.bin
rm -f ./fwimage.cfg

echo "*************************CheckSize *****************************"	
../../psp_rel/tools/maker_py/FW2X.exe -fsss $FW_NAME.FW MakeBin FLASHSIZE 123

grep "ERROR!! Firmware size is BIGGER than flash size" ./build.log
if [ $? == 0 ]
then
    echo "build error-->exit"
    exit 1;    
fi

echo "*************************Build UPGRADE.HEX*****************************"
../../psp_rel/tools/maker_py/FW2X.exe -fssf  $FW_NAME.FW  MakeBin MakeUPG $FW_NAME.FWU
cp -f $FW_NAME.FWU $FW_NAME.HEX
cp -f ./$FW_NAME.FWU ../ap_test/ATT/UPGRADE.HEX

echo "*************************Build ATF*****************************"
cd ../ap_test/ATT
pwd
make clean -f Makefile
make -f Makefile

cp -f actionstest.atf ../../fwpkg/$FW_NAME.atf  

cd ../../fwpkg

#判断是否支持OTA功能，支持OTA功能的固件才生成OTA文件
if [ $SupportOTA == 1 ]
then
    echo "*************************Build OTA*****************************"
    ../../psp_rel/tools/maker_py/FW2X.exe -fssf  $FW_NAME.FW  MakeBin MakeOTA $FW_NAME.OTA 
fi

	
