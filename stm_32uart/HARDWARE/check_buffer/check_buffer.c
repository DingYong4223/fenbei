#include "led.h"
#include "delay.h"
#include "sys.h"
#include "check_buffer.h"

//extern char Address=0x01;               //本机地址
//extern char Main_Address = 0x00;        //主机地址
extern Wire24G_buff Wire24G_Data[];   //控制数据二维数组
extern circle_buffer buffer;

extern u8 Tx_date_Buffer[8];  //需要发送的数据（发送缓冲器）
extern u8 Tx_Temp_Buffer[8];             //发送数据区（发送前数据）
extern u8 Rx_Temp_Buffer[18]; //接收处理缓冲数据区--默认的2.4G无线模块参数
extern u8 Rx_Config_Buffer[18]; //查询通信模块配置
//UINT8 Rx_date[3]={0x00,0x00,0x00};      //中控-无线传输过来的数据开关信息
//UINT8 RGB_date[3];                      //中控-无线传输过来的数据氛围灯
u8 Pwm_date;                         //中控-无线传输过来的数据--调光灯
u8 Send_Cnt;                  //发送数据延时计数
	
extern char rev_cnt0;
extern char Changer_Flag,Changer_Address;
extern char Main_Address,Address;
extern char Send_Flag,RGB_Flag,Setup_Flag;//,First_Flag=1;            //数据改变、需要发送标志位Date_Flag,,Pwm_Flag
//extern struct Wire24G_Dat;

u8 bufferPop(unsigned char *_buf)       //循环数据读数据
{
    if(buffer.head_pos==buffer.tail_pos)        //?????????????
        return 1; 
    else
    {
        *_buf=buffer.circle_buffer[buffer.head_pos];    //???????????????????
			  buffer.buffer_Cnt--;
        if(++buffer.head_pos>=Rx_Buffer_Max)
            buffer.head_pos=0;
				return 0;
    }
}

void bufferPush(const unsigned char _buf)        //循环数据写数据
{   
    buffer.circle_buffer[buffer.tail_pos]=_buf; //?????
	  buffer.buffer_Cnt++;
    if(++buffer.tail_pos>=Rx_Buffer_Max)           //?????
        buffer.tail_pos=0;                      //???????? ?? ??????
        if(buffer.tail_pos==buffer.head_pos)    //???????????? ????????????????
        if(++buffer.head_pos>=Rx_Buffer_Max)
            buffer.head_pos=0;

}

//void check_buffer(unsigned char rev_cnt0)
void check_buffer(void)
{
	u8 i,dat,Sum;

	
	if(0 == bufferPop(&dat))            //接收到数据
  {
	 	 __NOP();__NOP();__NOP();__NOP();   //延时0.66us
		 __NOP();__NOP();__NOP();__NOP();
		 __NOP();__NOP();__NOP();__NOP();
		 __NOP();__NOP();__NOP();__NOP();
		 __NOP();__NOP();__NOP();__NOP();
		 __NOP();__NOP();__NOP();__NOP();

 switch (rev_cnt0)
	{
  	case 0:
			if(dat == 0xFF)        //包头1位
			{
		  	Rx_Temp_Buffer[0] = dat;//	Rev_Buff0[0] = dat;
				rev_cnt0=1;
			}
			
  		break;
  	case 1:
			if(dat == 0xFE)        //包头2位
			{
				Rx_Temp_Buffer[1] =dat; //Rev_Buff0[1] = dat;
				rev_cnt0 = 2;
				
			}
			else if(dat == 0xFF)    //不是包头2位--是否为包头1位？
			{
				Rx_Temp_Buffer[0] = dat;//[0] = dat;
				rev_cnt0=1;
			}	
			else
			{
				rev_cnt0 = 0;     //不符合要求---重新包头检查-FF
			}
  		break;
		case 2:
			if(dat == Main_Address)     //包头3位==主控命令--地址 
			{
				Rx_Temp_Buffer[2] =dat;    //Rev_Buff0[1] = dat;
				rev_cnt0 = 3;
				
			}
			else if(dat == 0xFF)    //不是3位--是否包头1位？
			{
				Rx_Temp_Buffer[0] = dat;  //Rev_Buff0[0] = dat;
				rev_cnt0=1;
			}	
			else
			{
				rev_cnt0 = 0;           //不符合要求---重新包头检查-FF
			}
  		break;
	/*	case 3:
			if((dat&0x1F) == Address)     //本机地址
			{
				Rx_Temp_Buffer[3] =dat; //Rev_Buff0[1] = dat;
				rev_cnt0 = 4;
				
			}
			else if(dat == 0xFF)    //不是3位--是否是包头 ？-FF
			{
				Rx_Temp_Buffer[0] = dat;  //Rev_Buff0[0] = dat;
				rev_cnt0=1;
			}	
			else
			{
				rev_cnt0 = 0;        //不符合要求---重新包头检查-FF
			}
  		break;
		*/	
  	default://找到包头
			/////////////////////////////////////
			Rx_Temp_Buffer[rev_cnt0++] = dat;       //下一位数据
	   
			if (rev_cnt0 >= 8)
			{
				Sum = 0; //校验和清零
				for(i=0;i<7;i++)
				{
					Sum += Rx_Temp_Buffer[i];
				}
				if(Sum == Rx_Temp_Buffer[7])  //校验和--正确
				{				
					rev_cnt0 = 0;               //准备下一次
					
					i=0;                        //定位数据地址
			    while(Rx_Temp_Buffer[3] != Wire24G_Data[i].Wire24G_Addrees){
						i++;
						if(i==Wire24G_Max)    //地址底部-没有找到数据地址（地址错误）
						 break;
			     }
					if((Wire24G_Data[i].Data_value1==Rx_Temp_Buffer[4])&&(Wire24G_Data[i].Data_value2==Rx_Temp_Buffer[5])&&(Wire24G_Data[i].Data_value3==Rx_Temp_Buffer[6])){
					//数据与表中数据一致--没有改变 
 	       	 __NOP();__NOP();__NOP();__NOP();
					}
					else { //数据改变
					  Wire24G_Data[i].Data_value1=Rx_Temp_Buffer[4];
					  Wire24G_Data[i].Data_value2=Rx_Temp_Buffer[5];
					  Wire24G_Data[i].Data_value3=Rx_Temp_Buffer[6];
						Changer_Flag=1;                 //改变标志
						Changer_Address=i;              //改变地址						
						}							
		       
				}
				else         
				{ 	//校验和错误--检查后4位是否有包头
					if((Rx_Temp_Buffer[4] == 0xFF) && (Rx_Temp_Buffer[5] == 0xFE)&&(Rx_Temp_Buffer[6] == Main_Address) ){
				 Rx_Temp_Buffer[0] = 0xFF;
         Rx_Temp_Buffer[1] = 0xFE;
         Rx_Temp_Buffer[2] = Main_Address;
				 Sum = Rx_Temp_Buffer[7];
	       Rx_Temp_Buffer[3] = Sum;					
				 rev_cnt0 = 4;
					break;
					}
					//校验和错误--检查后3位是否有包头
				 else if ((Rx_Temp_Buffer[5] == 0xFF) && (Rx_Temp_Buffer[6] == 0xFE)&&(Rx_Temp_Buffer[7] == Main_Address)){  //新包头AA 5A 00
          Rx_Temp_Buffer[0] = 0xFF;
          Rx_Temp_Buffer[1] = 0xFE;
          Rx_Temp_Buffer[2] = Main_Address;
				  rev_cnt0 = 3;
					break;
				}
				 //校验和错误--检查后2位是否有包头
		     if ((Rx_Temp_Buffer[6] == 0xFF) && (Rx_Temp_Buffer[7] == 0xFE)){     //新包头FF FE
         Rx_Temp_Buffer[0] = 0xFF;
         Rx_Temp_Buffer[1] = 0xFE;
				 rev_cnt0 = 2;
					 break;
				 }
						 //校验和错误--检查后1位是否有包头		 
			  if ( Rx_Temp_Buffer[7] == 0xFF ){     //新包头FF
         Rx_Temp_Buffer[0] = 0xFF;
				 rev_cnt0 = 1;
					break;
				 }
				else 
					rev_cnt0 = 0;	               //新开始查找包头
			 }
				
		  }
			////////////////////////////////////
  		break;
	 }
  }
}
