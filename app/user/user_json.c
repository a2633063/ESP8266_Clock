#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"

#include "user_config.h"
#include "../cJson/cJSON.h"
#include "user_wifi.h"

#include "user_update.h"
#include "user_json.h"
#include "user_setting.h"
#include "user_function.h"



void ICACHE_FLASH_ATTR user_json_analysis(bool udp_flag, u8* jsonRoot) {
	uint8_t i;
	bool update_user_config_flag = false;   //��־λ,��¼����Ƿ���Ҫ���´��������
	uint8_t plug_retained = 0;
	cJSON *pJsonRoot = cJSON_Parse(jsonRoot);	//���������ж��Ƿ�Ϊһ��json��ʽ������
	//����Ƿ�json��ʽ����
	if (pJsonRoot != NULL) {

		//���ڴ�ӡ����
//		char *s = cJSON_Print(pJsonRoot);
//		os_printf("pJsonRoot: %s\r\n", s);
//		cJSON_free((void *) s);

//����device report
		os_printf("start json:\r\n");
		cJSON *p_cmd = cJSON_GetObjectItem(pJsonRoot, "cmd");
		if (p_cmd && cJSON_IsString(p_cmd) && os_strcmp(p_cmd->valuestring, "device report") == 0) {

			os_printf("device report\r\n");
			cJSON *pRoot = cJSON_CreateObject();
			cJSON_AddStringToObject(pRoot, "name", user_config.name);
			cJSON_AddStringToObject(pRoot, "ip", strIP);
			cJSON_AddStringToObject(pRoot, "mac", strMac);
			cJSON_AddNumberToObject(pRoot, "type", TYPE);
			cJSON_AddStringToObject(pRoot, "type_name", TYPE_NAME);
			char *s = cJSON_Print(pRoot);
			os_printf("pRoot: %s\r\n", s);

			user_send(udp_flag, s, 0);

			cJSON_free((void *) s);
			cJSON_Delete(pRoot);
		}

		//����
		cJSON *p_name = cJSON_GetObjectItem(pJsonRoot, "name");
		cJSON *p_mac = cJSON_GetObjectItem(pJsonRoot, "mac");

		//
		if ((p_name && cJSON_IsString(p_name) && os_strcmp(p_name->valuestring, user_config.name) == 0) //name
		|| (p_mac && cJSON_IsString(p_mac) && os_strcmp(p_mac->valuestring, strMac) == 0)	//mac
				) {

			cJSON *json_send = cJSON_CreateObject();
			cJSON_AddStringToObject(json_send, "mac", strMac);

			//�����汾
			cJSON *p_version = cJSON_GetObjectItem(pJsonRoot, "version");
			if (p_version) {
				cJSON_AddStringToObject(json_send, "version", VERSION);
			}

			//����wifi ssid
			cJSON *p_ssid = cJSON_GetObjectItem(pJsonRoot, "ssid");
			if (p_ssid) {
				struct station_config ssidGet;
				if (wifi_station_get_config_default(&ssidGet)) {
					cJSON_AddStringToObject(json_send, "ssid", ssidGet.ssid);
				} else {
					cJSON_AddStringToObject(json_send, "ssid", "get wifi_ssid fail");
				}
			}




			cJSON *p_setting = cJSON_GetObjectItem(pJsonRoot, "setting");
			if (p_setting) {

				//����ota
				uint8_t userBin = system_upgrade_userbin_check();
				cJSON *p_ota1 = cJSON_GetObjectItem(p_setting, "ota1");
				cJSON *p_ota2 = cJSON_GetObjectItem(p_setting, "ota2");
				if (userBin == UPGRADE_FW_BIN2) {
					if (p_ota1 && cJSON_IsString(p_ota1)) {
						if (cJSON_IsString(p_ota1))
							user_ota_start(p_ota1->valuestring);
					}
				} else {
					if (p_ota2 && cJSON_IsString(p_ota2)) {
						if (cJSON_IsString(p_ota2))
							user_ota_start(p_ota2->valuestring);
					}
				}

				//�����豸����
				cJSON *p_setting_name = cJSON_GetObjectItem(p_setting, "name");
				if (p_setting_name && cJSON_IsString(p_setting_name)) {
					update_user_config_flag = true;
					os_sprintf(user_config.name, p_setting_name->valuestring);
				}

				//����wifi ssid
				cJSON *p_setting_wifi_ssid = cJSON_GetObjectItem(p_setting, "wifi_ssid");
				cJSON *p_setting_wifi_password = cJSON_GetObjectItem(p_setting, "wifi_password");
				if (p_setting_wifi_ssid && cJSON_IsString(p_setting_wifi_ssid) && p_setting_wifi_password
						&& cJSON_IsString(p_setting_wifi_password)) {

					user_wifi_set(p_setting_wifi_ssid->valuestring, p_setting_wifi_password->valuestring);
//					struct station_config stationConf;
//					stationConf.bssid_set = 0; //need not check MAC address of AP
//					os_sprintf(stationConf.ssid, p_setting_wifi_ssid->valuestring);
//					os_sprintf(stationConf.password, p_setting_wifi_password->valuestring);
//					wifi_station_set_config(&stationConf);
				}

				//����mqtt ip
				cJSON *p_mqtt_ip = cJSON_GetObjectItem(p_setting, "mqtt_uri");
				if (p_mqtt_ip && cJSON_IsString(p_mqtt_ip)) {
					update_user_config_flag = true;
					os_sprintf(user_config.mqtt_ip, p_mqtt_ip->valuestring);
				}

				//����mqtt port
				cJSON *p_mqtt_port = cJSON_GetObjectItem(p_setting, "mqtt_port");
				if (p_mqtt_port && cJSON_IsNumber(p_mqtt_port)) {
					update_user_config_flag = true;
					user_config.mqtt_port = p_mqtt_port->valueint;
				}

				//����mqtt user
				cJSON *p_mqtt_user = cJSON_GetObjectItem(p_setting, "mqtt_user");
				if (p_mqtt_user && cJSON_IsString(p_mqtt_user)) {
					update_user_config_flag = true;
					os_sprintf(user_config.mqtt_user, p_mqtt_user->valuestring);
				}

				//����mqtt password
				cJSON *p_mqtt_password = cJSON_GetObjectItem(p_setting, "mqtt_password");
				if (p_mqtt_password && cJSON_IsString(p_mqtt_password)) {
					update_user_config_flag = true;
					os_sprintf(user_config.mqtt_password, p_mqtt_password->valuestring);
				}

				//��ʼ��������
				cJSON *json_setting_send = cJSON_CreateObject();
				//�����豸ota
				if (p_ota1)
					cJSON_AddStringToObject(json_setting_send, "ota1", p_ota1->valuestring);
				if (p_ota2)
					cJSON_AddStringToObject(json_setting_send, "ota2", p_ota2->valuestring);
				//�����豸����
				if (p_setting_name)
					cJSON_AddStringToObject(json_setting_send, "name", user_config.name);

				//�����豸wifi
				if (p_setting_wifi_ssid || p_setting_wifi_password) {
					struct station_config configGet;
					if (wifi_station_get_config_default(&configGet)) {
						cJSON_AddStringToObject(json_setting_send, "wifi_ssid", configGet.ssid);
						cJSON_AddStringToObject(json_setting_send, "wifi_password", configGet.password);
					} else {
						cJSON_AddStringToObject(json_setting_send, "wifi_ssid", "get wifi_ssid fail");
						cJSON_AddStringToObject(json_setting_send, "wifi_password", "get wifi_password fail");
					}
				}

				//����mqtt ip
				if (p_mqtt_ip)
					cJSON_AddStringToObject(json_setting_send, "mqtt_uri", user_config.mqtt_ip);

				//����mqtt port
				if (p_mqtt_port)
					cJSON_AddNumberToObject(json_setting_send, "mqtt_port", user_config.mqtt_port);

				//����mqtt user
				if (p_mqtt_user)
					cJSON_AddStringToObject(json_setting_send, "mqtt_user", user_config.mqtt_user);

				//����mqtt password
				if (p_mqtt_password)
					cJSON_AddStringToObject(json_setting_send, "mqtt_password", user_config.mqtt_password);

				cJSON_AddItemToObject(json_send, "setting", json_setting_send);

				if ((p_mqtt_ip && cJSON_IsString(p_mqtt_ip) && p_mqtt_port && cJSON_IsNumber(p_mqtt_port) && p_mqtt_user
						&& cJSON_IsString(p_mqtt_user) && p_mqtt_password && cJSON_IsString(p_mqtt_password) && !user_mqtt_is_connect())) {
					system_restart();
				}
			}

			cJSON_AddStringToObject(json_send, "name", user_config.name);

			char *json_str = cJSON_Print(json_send);
			os_printf("json_send: %s\r\n", json_str);
			user_send(udp_flag, json_str, plug_retained);
			cJSON_free((void *) json_str);

			if (update_user_config_flag) {
				user_setting_set_config();
				update_user_config_flag = false;
			}

			cJSON_Delete(json_send);
		}

	} else {
		os_printf("this is not a json data:\r\n%s\r\n", jsonRoot);
	}

	cJSON_Delete(pJsonRoot);
//	os_printf("get freeHeap2: %d \n\n", system_get_free_heap_size());
}
