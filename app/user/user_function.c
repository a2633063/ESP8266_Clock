#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"

#include "espconn.h"

#include "user_function.h"
#include "user_setting.h"
#include "user_wifi.h"
#include "user_tm1628.h"
#include "user_alarm.h"
/*
 * ͨ�ú���
 * tcp udp�������ݺ�Ĵ�����
 * �ṹ: a5 5a length mac[5] +cmd+....
 */

void ICACHE_FLASH_ATTR
user_con_received(void *arg, char *pusrdata, unsigned short length) {
	if (length == 1 && *pusrdata == 127)
		return;

	struct espconn *pesp_conn = arg;

	int i, j, k;
	char DeviceBuffer[40] = { 0 };

	os_sprintf(DeviceBuffer, "result:%s\n", pusrdata);

	if (pesp_conn->type == ESPCONN_TCP) { //tcp
		//espconn_sent(pesp_conn, DeviceBuffer, os_strlen(DeviceBuffer));
	} else if (pesp_conn->type == ESPCONN_UDP) {		//udp
		remot_info *premot = NULL;
		if (espconn_get_connection_info(pesp_conn, &premot, 0) != ESPCONN_OK)
			return;
		pesp_conn->proto.tcp->remote_port = premot->remote_port;	//��ȡ�˿�
		pesp_conn->proto.tcp->remote_ip[0] = premot->remote_ip[0];	//��ȡIP��ַ
		pesp_conn->proto.tcp->remote_ip[1] = premot->remote_ip[1];
		pesp_conn->proto.tcp->remote_ip[2] = premot->remote_ip[2];
		pesp_conn->proto.tcp->remote_ip[3] = premot->remote_ip[3];
		//espconn_sent(pesp_conn, DeviceBuffer, os_strlen(DeviceBuffer));	//��������
	}
	if(length>4 && (MacAddr[5]==*(pusrdata+3)||*(pusrdata+3)==0xff) && length==*(pusrdata+2)
			&& *(pusrdata)==0xa5&& *(pusrdata+1)==0x5a){	//�����㹻֮ǰ��mac��ַƥ��ʱ�Ŵ�������

		char Device_buffer[60] = { 0 };
		switch(*(pusrdata+4)){
			case 0:
			{	//��ѯMAC��ַ��IP��ַ
				struct ip_info ipconfig;
				wifi_get_ip_info(STATION_IF, &ipconfig);

				os_memcpy(Device_buffer+5,MacAddr,6);
				os_memcpy(Device_buffer+11,&(ipconfig.ip),4);

				 Device_buffer[0]=0xa5;
				 Device_buffer[1]=0x5a;
				 Device_buffer[2]=15;
				 Device_buffer[3]=MacAddr[5];
				 Device_buffer[4]=*(pusrdata+4);
				int i=0;
				os_printf("send Data:");
				for(i=0;i<15;i++){
					os_printf("%X ",*(Device_buffer+i));
				}
				os_printf("\n");
				espconn_sent(pesp_conn, Device_buffer, 15);
			}
				break;
			case 1:
			{	//��������
				if(length<6)break;
				unsigned char light =*(pusrdata+5);
				if(light != 0xff){
					user_beep_on(100);
					user_tm1628_set_brightness(light);
				}
				Device_buffer[0]=0xa5;
				Device_buffer[1]=0x5a;
				Device_buffer[2]=6;
				Device_buffer[3]=MacAddr[5];
				Device_buffer[4]=*(pusrdata+4);
				if(auto_brightness == 0)
					Device_buffer[5]=brightness;
				else Device_buffer[5]=8;
				espconn_sent(pesp_conn, Device_buffer, 6);
			}
			break;
			case 2:
			{	//���������ܿ���
				if(length<6)break;
				unsigned char temp =*(pusrdata+5);
				if(temp==0||temp==1) {
					alarm_main_switch=temp;
					user_setting_save_alarm_switch();
				}
				Device_buffer[0]=0xa5;
				Device_buffer[1]=0x5a;
				Device_buffer[2]=6;
				Device_buffer[3]=MacAddr[5];
				Device_buffer[4]=*(pusrdata+4);
				Device_buffer[5]=alarm_main_switch;
				espconn_sent(pesp_conn, Device_buffer, 6);
			}
			break;
			case 3:
			{	//��������
				if(length==5){
					//��ѯ��������
					int i=0;
					for(i=0;i<ALARM_COUNT;i++){
					os_memcpy(Device_buffer+6,&alarm[i],sizeof(struct struct_alarm));
					Device_buffer[0]=0xa5;
					Device_buffer[1]=0x5a;
					Device_buffer[2]=10;
					Device_buffer[3]=MacAddr[5];
					Device_buffer[4]=*(pusrdata+4);
					Device_buffer[5]=i;
					espconn_sent(pesp_conn, Device_buffer, 10);
					}
					break;
				}

				unsigned char no=*(pusrdata+5);	//���ӱ��
				if(no>=ALARM_COUNT) break;
				if(length>=10){
					alarm[no].on=*(pusrdata+6);	//���ӿ���
					alarm[no].repeat=*(pusrdata+7);	//�����ظ�
					alarm[no].hour=*(pusrdata+8);	//����ʱ
					alarm[no].minute=*(pusrdata+9);	//���ӷ�
					//TODO: ������������
					user_setting_save_alarm(no);
				}

				os_memcpy(Device_buffer+6,&alarm[no],sizeof(struct struct_alarm));

				Device_buffer[0]=0xa5;
				Device_buffer[1]=0x5a;
				Device_buffer[2]=10;
				Device_buffer[3]=MacAddr[5];
				Device_buffer[4]=*(pusrdata+4);
				Device_buffer[5]=no;

				espconn_sent(pesp_conn, Device_buffer, 10);


			}
			break;
		}

	}


	/*
	if (length == os_strlen(device_find_request) &&
	os_strncmp(pusrdata, device_find_request, os_strlen(device_find_request)) == 0) {

		char Device_mac_buffer[60] = { 0 };
		char hwaddr[6];
		struct ip_info ipconfig;

		wifi_get_ip_info(STATION_IF, &ipconfig);
		wifi_get_macaddr(STATION_IF, hwaddr);

		os_sprintf(DeviceBuffer, "%s" MACSTR "," IPSTR, device_find_response_ok, MAC2STR(hwaddr), IP2STR(&ipconfig.ip));
		os_printf("%s\n", DeviceBuffer);
		espconn_sent(pesp_conn, DeviceBuffer, os_strlen(DeviceBuffer));
	}   else if (length == os_strlen(setting_update)
			&& os_strncmp(pusrdata, setting_update, os_strlen(setting_update)) == 0) {
		os_printf("\nupdate\n");
		os_printf("user bin:%d\n", system_upgrade_userbin_check());
//		user_update();
	}	else{
		int i=0;
		os_printf("get Data:");
		for(i=0;i<length;i++){
			os_printf("%X ",*(pusrdata+i));
		}
		os_printf("\n");
	}*/

}
