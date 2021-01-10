//#include "led.h"
//#include "delay.h"
#include "sys.h"
//#include "usart.h"

#define Rx_Buffer_Max 0x80
#define Wire24G_Max 0x30   //62���ڵ�

typedef struct _circle_buffer{
    unsigned char head_pos;             //����ͷ��λ��
    unsigned char tail_pos;             //����β��λ�� 
    u8 buffer_Cnt;                   //������   
    u8 circle_buffer[Rx_Buffer_Max];  //��������
}circle_buffer;

typedef struct _Wire24G_buff{
    unsigned char serial_num;             //�������������
    unsigned char Wire24G_Addrees;        //����2.4G�������ݵ�ַ
    u8 DatatypeProperty;                  //��������
    u8 Data_value1;                       //��������
	  u8 Data_value2;
	  u8 Data_value3;
	  u8 Correlation_num;                   //�������
}Wire24G_buff;

   //�������ݶ�ά����

unsigned char bufferPop(unsigned char *);
extern void bufferPush(unsigned char );
void check_buffer(void);
