#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define BCDtoDEC(x) ( ((x)>>4)*10+(x)%0x10  )           //BCD��ת��Ϊʮ���Ʊ�ʾ��ʽ
#define DECtoBCD(x) ( (((x)/10)<<4)+(x)%10  )           //ʮ����ת��ΪBCD���ʾ��ʽ


#endif

