#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "os_type.h"

#define BCDtoDEC(x) ( ((x)>>4)*10+(x)%0x10  )           //BCD��ת��Ϊʮ���Ʊ�ʾ��ʽ
#define DECtoBCD(x) ( (((x)/10)<<4)+(x)%10  )           //ʮ����ת��ΪBCD���ʾ��ʽ

#define VERSION "v0.0.2"

#define TYPE 6
#define TYPE_NAME "zClock"

#define DEVICE_NAME "zClock_%02X%02X"
#define MDNS_DEVICE_NAME "zClock_%s"

#define USER_CONFIG_VERSION 1

#define SETTING_MQTT_STRING_LENGTH_MAX  64      //���� 4 �ֽڶ��롣
#define NAME_LENGTH 32		//���������ַ�����󳤶�




//�û���������ṹ��
typedef struct {
	char version;
	uint8_t name[NAME_LENGTH];
	uint8_t mqtt_ip[SETTING_MQTT_STRING_LENGTH_MAX];   //mqtt service ip
	uint16_t mqtt_port;        //mqtt service port
	uint8_t mqtt_user[SETTING_MQTT_STRING_LENGTH_MAX];     //mqtt service user
	uint8_t mqtt_password[SETTING_MQTT_STRING_LENGTH_MAX];     //mqtt service user
	uint8_t direction;

} user_config_t;

extern char rtc_init;
extern user_config_t user_config;

#endif

