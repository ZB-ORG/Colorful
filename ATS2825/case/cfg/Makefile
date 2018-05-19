#定义伪目标
 
all:

	make -C ./../../psp_rel/usermodule/enhanced/make_build
	make -C ./../drv/welcome
	make -C ./../drv/Key_boombox
	make -C ./../drv/LED_7SEG
	make -C ./../drv/fm_qn8035
	make -C ./../drv/ccd
	make -C ./../ap/common
	make -C ./strip_lib   #必须在make enhanced 和 common 之后
	make -C ./../ap/ap_manager
	make -C ./../ap/ap_music
	make -C ./../ap/ap_alarm
	make -C ./../ap/ap_config
	make -C ./../ap/music_engine
	make -C ./../ap/music_scan	
	make -C ./../ap/ap_linein
	make -C ./../ap/linein_engine
	make -C ./../ap/ap_usound	
	make -C ./../ap/ap_cardreader	
	make -C ./../ap/fm_engine
	make -C ./../ap/ap_radio	
	make -C ./../ap/btplay_engine
	make -C ./../ap/btcall_engine
	make -C ./../ap/ap_btplay
	make -C ./../ap/ap_btcall
	make -C ./../ap/ap_upgrade	
	make -C ./../ap/usound_engine	
clean:
	make -C ./../../psp_rel/usermodule/enhanced/make_build clean
	make -C ./../drv/welcome clean
	make -C ./../drv/Key_boombox clean
	make -C ./../drv/LED_7SEG clean
	make -C ./../drv/fm_qn8035 clean	
	make -C ./../drv/ccd clean
	make -C ./../ap/common clean
	make -C ./../ap/ap_manager clean
	make -C ./../ap/ap_music clean
	make -C ./../ap/ap_alarm clean
	make -C ./../ap/music_scan clean	
	make -C ./../ap/ap_config clean
	make -C ./../ap/music_engine clean
	make -C ./../ap/ap_linein clean
	make -C ./../ap/linein_engine clean
	make -C ./../ap/ap_usound clean
	make -C ./../ap/ap_cardreader clean		
	make -C ./../ap/fm_engine clean
	make -C ./../ap/ap_radio clean
	make -C ./../ap/btplay_engine clean
	make -C ./../ap/btcall_engine clean
	make -C ./../ap/ap_btplay clean
	make -C ./../ap/ap_btcall clean
	make -C ./../ap/ap_upgrade clean		
	make -C ./../ap/usound_engine	clean		
clean_target:	
	make -C ./../ap/common clean_target
	make -C ./../ap/ap_manager clean_target
	make -C ./../ap/ap_music clean_target
	make -C ./../ap/ap_alarm clean_target
	make -C ./../ap/ap_config clean_target
	make -C ./../ap/btplay_engine clean_target
	make -C ./../ap/btcall_engine clean_target
	make -C ./../ap/ap_btplay clean_target
	make -C ./../ap/ap_btcall clean_target
	make -C ./../ap/music_engine clean_target
	make -C ./../ap/music_scan clean_target	
	make -C ./../ap/ap_linein clean_target
	make -C ./../ap/linein_engine clean_target
	make -C ./../ap/ap_cardreader clean_target		
	make -C ./../ap/fm_engine clean_target
	make -C ./../ap/ap_radio clean_target
	make -C ./../ap/ap_upgrade clean_target	
	make -C ./../ap/ap_usound clean_target
	make -C ./../ap/usound_engine clean_target
