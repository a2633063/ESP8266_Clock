/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
 *******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"

#include "user_key.h"
#include "user_led.h"
#include "user_wifi.h"
#include "user_sntp.h"
#include "user_tm1628.h"
#include "user_devicefind.h"
#include "user_ds3231.h"

void user_rf_pre_init(void) {
//	system_set_os_print(0);	//�ر�log��ӡ����
}



void system_init_done(void) {
	//gpio16_input_conf();	//���ð���GPIO16Ϊ��� user_key_init���Ѿ���ʼ��

	if(gpio16_input_get()==0){
		os_printf("system init with KEY pressed,start smartconfig\n");
		user_smartconfig();
	}
}
/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void user_init(void) {
	int i, j;
	uint32 x;

	uart_init(115200, 115200);
	os_printf("\n\nStart user%d.bin\n", system_upgrade_userbin_check() + 1);
	os_printf("SDK version:%s\n", system_get_sdk_version());


	user_wifi_init();
	user_key_init();
	user_led_init();
	user_ds3231_init();
	user_sntp_init();
	user_tm1628_init();


	//UDP��ʼ��,�����˿�12345,�����յ��ض��ַ���ʱ,���ر��豸IP��MAC��ַ
	user_devicefind_init(12345);

//	char ssid[32] = "Honor 8";
//	char password[64] = "hyj19910911";
//	struct station_config stationConf;
//	stationConf.bssid_set = 0; //need not check MAC address of AP
//	os_memcpy(&stationConf.ssid, ssid, 32);
//	os_memcpy(&stationConf.password, password, 64);
//	wifi_station_set_config(&stationConf);


	system_init_done_cb(system_init_done);
}

