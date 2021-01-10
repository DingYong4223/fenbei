#include "led.h"
#include "delay.h"
#include "sys.h"
#include "check_buffer.h"

//extern char Address=0x01;               //������ַ
//extern char Main_Address = 0x00;        //������ַ
extern Wire24G_buff Wire24G_Data[];   //�������ݶ�ά����
extern circle_buffer buffer;

extern u8 Tx_date_Buffer[8];  //��Ҫ���͵����ݣ����ͻ�������
extern u8 Tx_Temp_Buffer[8];             //����������������ǰ���ݣ�
extern u8 Rx_Temp_Buffer[18]; //���մ�����������--Ĭ�ϵ�2.4G����ģ�����
extern u8 Rx_Config_Buffer[18]; //��ѯͨ��ģ������
//UINT8 Rx_date[3]={0x00,0x00,0x00};      //�п�-���ߴ�����������ݿ�����Ϣ
//UINT8 RGB_date[3];                      //�п�-���ߴ�����������ݷ�Χ��
u8 Pwm_date;                         //�п�-���ߴ������������--�����
u8 Send_Cnt;                  //����������ʱ����
	
extern char rev_cnt0;
extern char Changer_Flag,Changer_Address;
extern char Main_Address,Address;
extern char Send_Flag,RGB_Flag,Setup_Flag;//,First_Flag=1;            //���ݸı䡢��Ҫ���ͱ�־λDate_Flag,,Pwm_Flag
//extern struct Wire24G_Dat;

u8 bufferPop(unsigned char *_buf)       //ѭ�����ݶ�����
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

void bufferPush(const unsigned char _buf)        //ѭ������д����
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

	
	if(0 == bufferPop(&dat))            //���յ�����
  {
	 	 __NOP();__NOP();__NOP();__NOP();   //��ʱ0.66us
		 __NOP();__NOP();__NOP();__NOP();
		 __NOP();__NOP();__NOP();__NOP();
		 __NOP();__NOP();__NOP();__NOP();
		 __NOP();__NOP();__NOP();__NOP();
		 __NOP();__NOP();__NOP();__NOP();

 switch (rev_cnt0)
	{
  	case 0:
			if(dat == 0xFF)        //��ͷ1λ
			{
		  	Rx_Temp_Buffer[0] = dat;//	Rev_Buff0[0] = dat;
				rev_cnt0=1;
			}
			
  		break;
  	case 1:
			if(dat == 0xFE)        //��ͷ2λ
			{
				Rx_Temp_Buffer[1] =dat; //Rev_Buff0[1] = dat;
				rev_cnt0 = 2;
				
			}
			else if(dat == 0xFF)    //���ǰ�ͷ2λ--�Ƿ�Ϊ��ͷ1λ��
			{
				Rx_Temp_Buffer[0] = dat;//[0] = dat;
				rev_cnt0=1;
			}	
			else
			{
				rev_cnt0 = 0;     //������Ҫ��---���°�ͷ���-FF
			}
  		break;
		case 2:
			if(dat == Main_Address)     //��ͷ3λ==��������--��ַ 
			{
				Rx_Temp_Buffer[2] =dat;    //Rev_Buff0[1] = dat;
				rev_cnt0 = 3;
				
			}
			else if(dat == 0xFF)    //����3λ--�Ƿ��ͷ1λ��
			{
				Rx_Temp_Buffer[0] = dat;  //Rev_Buff0[0] = dat;
				rev_cnt0=1;
			}	
			else
			{
				rev_cnt0 = 0;           //������Ҫ��---���°�ͷ���-FF
			}
  		break;
	/*	case 3:
			if((dat&0x1F) == Address)     //������ַ
			{
				Rx_Temp_Buffer[3] =dat; //Rev_Buff0[1] = dat;
				rev_cnt0 = 4;
				
			}
			else if(dat == 0xFF)    //����3λ--�Ƿ��ǰ�ͷ ��-FF
			{
				Rx_Temp_Buffer[0] = dat;  //Rev_Buff0[0] = dat;
				rev_cnt0=1;
			}	
			else
			{
				rev_cnt0 = 0;        //������Ҫ��---���°�ͷ���-FF
			}
  		break;
		*/	
  	default://�ҵ���ͷ
			/////////////////////////////////////
			Rx_Temp_Buffer[rev_cnt0++] = dat;       //��һλ����
	   
			if (rev_cnt0 >= 8)
			{
				Sum = 0; //У�������
				for(i=0;i<7;i++)
				{
					Sum += Rx_Temp_Buffer[i];
				}
				if(Sum == Rx_Temp_Buffer[7])  //У���--��ȷ
				{				
					rev_cnt0 = 0;               //׼����һ��
					
					i=0;                        //��λ���ݵ�ַ
			    while(Rx_Temp_Buffer[3] != Wire24G_Data[i].Wire24G_Addrees){
						i++;
						if(i==Wire24G_Max)    //��ַ�ײ�-û���ҵ����ݵ�ַ����ַ����
						 break;
			     }
					if((Wire24G_Data[i].Data_value1==Rx_Temp_Buffer[4])&&(Wire24G_Data[i].Data_value2==Rx_Temp_Buffer[5])&&(Wire24G_Data[i].Data_value3==Rx_Temp_Buffer[6])){
					//�������������һ��--û�иı� 
 	       	 __NOP();__NOP();__NOP();__NOP();
					}
					else { //���ݸı�
					  Wire24G_Data[i].Data_value1=Rx_Temp_Buffer[4];
					  Wire24G_Data[i].Data_value2=Rx_Temp_Buffer[5];
					  Wire24G_Data[i].Data_value3=Rx_Temp_Buffer[6];
						Changer_Flag=1;                 //�ı��־
						Changer_Address=i;              //�ı��ַ						
						}							
		       
				}
				else         
				{ 	//У��ʹ���--����4λ�Ƿ��а�ͷ
					if((Rx_Temp_Buffer[4] == 0xFF) && (Rx_Temp_Buffer[5] == 0xFE)&&(Rx_Temp_Buffer[6] == Main_Address) ){
				 Rx_Temp_Buffer[0] = 0xFF;
         Rx_Temp_Buffer[1] = 0xFE;
         Rx_Temp_Buffer[2] = Main_Address;
				 Sum = Rx_Temp_Buffer[7];
	       Rx_Temp_Buffer[3] = Sum;					
				 rev_cnt0 = 4;
					break;
					}
					//У��ʹ���--����3λ�Ƿ��а�ͷ
				 else if ((Rx_Temp_Buffer[5] == 0xFF) && (Rx_Temp_Buffer[6] == 0xFE)&&(Rx_Temp_Buffer[7] == Main_Address)){  //�°�ͷAA 5A 00
          Rx_Temp_Buffer[0] = 0xFF;
          Rx_Temp_Buffer[1] = 0xFE;
          Rx_Temp_Buffer[2] = Main_Address;
				  rev_cnt0 = 3;
					break;
				}
				 //У��ʹ���--����2λ�Ƿ��а�ͷ
		     if ((Rx_Temp_Buffer[6] == 0xFF) && (Rx_Temp_Buffer[7] == 0xFE)){     //�°�ͷFF FE
         Rx_Temp_Buffer[0] = 0xFF;
         Rx_Temp_Buffer[1] = 0xFE;
				 rev_cnt0 = 2;
					 break;
				 }
						 //У��ʹ���--����1λ�Ƿ��а�ͷ		 
			  if ( Rx_Temp_Buffer[7] == 0xFF ){     //�°�ͷFF
         Rx_Temp_Buffer[0] = 0xFF;
				 rev_cnt0 = 1;
					break;
				 }
				else 
					rev_cnt0 = 0;	               //�¿�ʼ���Ұ�ͷ
			 }
				
		  }
			////////////////////////////////////
  		break;
	 }
  }
}
