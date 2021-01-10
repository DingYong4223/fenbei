//#include "led.h"
//#include "delay.h"
#include "sys.h"
//#include "usart.h"

#define Rx_Buffer_Max 0x80
#define Wire24G_Max 0x30   //62个节点

typedef struct _circle_buffer{
    unsigned char head_pos;             //缓冲头部位置
    unsigned char tail_pos;             //缓冲尾部位置 
    u8 buffer_Cnt;                   //数据亮   
    u8 circle_buffer[Rx_Buffer_Max];  //缓冲数据
}circle_buffer;

typedef struct _Wire24G_buff{
    unsigned char serial_num;             //缓冲区数据序号
    unsigned char Wire24G_Addrees;        //缓冲2.4G无线数据地址
    u8 DatatypeProperty;                  //数据属性
    u8 Data_value1;                       //控制数据
	  u8 Data_value2;
	  u8 Data_value3;
	  u8 Correlation_num;                   //关联编号
}Wire24G_buff;

   //控制数据二维数组

unsigned char bufferPop(unsigned char *);
extern void bufferPush(unsigned char );
void check_buffer(void);
