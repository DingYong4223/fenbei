#include "timer.h"
#include "led.h"
#include "check_buffer.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//ͨ�ö�ʱ�� ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/12/03
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  
 

//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
extern char Scan_Cnt,Main_Address,Send_Flag,Time_Cnt,RGB_Flag,Time_Flag;
extern Wire24G_buff Wire24G_Data[];   //�������ݶ�ά����
//extern Tx_date_Buffer[];

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //ʹ��
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
							 
}

void TIM3_IRQHandler(void)   //TIM3�ж�
{
//	u8 Sum,Temp;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ 
 
	/*	
			Sum = 0;           
			for(Temp=0;Temp<7;Temp++)
			 Sum += Tx_date_Buffer[Temp];      //����У���
			Tx_date_Buffer[7] = Sum;           //�������ݵ�У���
		for(Temp=0;Temp<7;Temp++)							//ѭ����������
	{
	  while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)== SET); //ѭ������,ֱ���������   
		USART_SendData(USART3,Tx_date_Buffer[Temp]);  //�Ѹ�ʽ���ַ����ӿ����崮���ͳ�ȥ 
	}	
			USART_SendData(USART3, 0x00);
			while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);
			USART_SendData(USART3, 0x01);
			USART_SendData(USART3, 0x02);
			Tes=0xfd+Wire24G_Data[Scan_Cnt].Wire24G_Addrees+Main_Address;
	//		 USART_SendData(USART3, Tes);
	//		}
*/
    Send_Flag =1;
		Time_Cnt ++;
		if (Time_Cnt == 0x20)
		{
			Time_Cnt = 0;
			RGB_Flag = 1;
			Time_Flag++;
			if (Time_Flag>3)Time_Flag =0;
		}

		LED0=!LED0;
		}
}












