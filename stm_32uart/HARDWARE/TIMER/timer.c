#include "timer.h"
#include "led.h"
#include "check_buffer.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//通用定时器 驱动代码			   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/12/03
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  
 

//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
extern char Scan_Cnt,Main_Address,Send_Flag,Time_Cnt,RGB_Flag,Time_Flag;
extern Wire24G_buff Wire24G_Data[];   //控制数据二维数组
//extern Tx_date_Buffer[];

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
							 
}

void TIM3_IRQHandler(void)   //TIM3中断
{
//	u8 Sum,Temp;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
 
	/*	
			Sum = 0;           
			for(Temp=0;Temp<7;Temp++)
			 Sum += Tx_date_Buffer[Temp];      //计算校验和
			Tx_date_Buffer[7] = Sum;           //发送数据的校验和
		for(Temp=0;Temp<7;Temp++)							//循环发送数据
	{
	  while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)== SET); //循环发送,直到发送完毕   
		USART_SendData(USART3,Tx_date_Buffer[Temp]);  //把格式化字符串从开发板串口送出去 
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












