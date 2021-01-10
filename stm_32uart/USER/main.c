#include <stdlib.h>
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "msg.h"
#include "round_queue.h"
#include "packet.h"
#include "esp8266.h"
#include "string.h"
#include "func.h"
#include "cJSON.h"
#include "check_buffer.h"
#include "timer.h"

extern u16 ESP8266_STA;
extern rcv_buffer_t rcv_buf;
extern Packet packetRecv;
extern Packet packetSend;
void parsePacket(void);
extern uint8_t at_buffer[];
extern char at_command[];
uint32_t no_conn_time_out=0;   //no connect time out.

char rev_cnt0;
char Scan_Cnt;
char Time_Cnt;
char Changer_Flag,Changer_Address;
char Address=0x01;               //本机地址
char Main_Address = 0x00;        //主机地址
char Send_Flag,RGB_Flag,Setup_Flag,Time_Flag;//,First_Flag=1;            //数据改变、需要发送标志位Date_Flag,,Pwm_Flag

Wire24G_buff Wire24G_Data[Wire24G_Max];   //控制数据二维数组

// struct Wire24G_buff[Wire24G_Max ]; 
circle_buffer buffer;

u8 Tx_date_Buffer[8]={0xFF,0xFE,0x00,0x01,0x02,0x03,0x04,0x01};  //需要发送的数据（发送缓冲器）
u8 Tx_Temp_Buffer[8];             //发送数据区（发送前数据）
u8 Rx_Temp_Buffer[18]  = {0xAA,0x5A,0x11,0x11,0x11,0x11,0x00,0x00,0x00,0x04,0x00,0x64,0x00,0x01,0x00,0x12,0x00,0xC3}; //接收处理缓冲数据区--默认的2.4G无线模块参数
u8 Rx_Config_Buffer[18]=	{0xAA,0x5C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06}; //查询通信模块配置

 int main(void){
//	 char destin[20]={0};
//	 char* macEnd;
	u8 Sum,Temp;
	u16 len;
	 
	 Wire24G_Data[1].Wire24G_Addrees=1;
	 Wire24G_Data[1].serial_num=1; 
	 Wire24G_Data[1].DatatypeProperty=0;
	 
	 Wire24G_Data[15].Wire24G_Addrees=2;   //RGB控制2#
	 Wire24G_Data[15].serial_num=15; 
	 Wire24G_Data[15].DatatypeProperty=3;
	 
	 Wire24G_Data[10].Wire24G_Addrees=6;
	 Wire24G_Data[10].serial_num=10; 
	 Wire24G_Data[10].DatatypeProperty=0;
	 
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	TIM3_Int_Init(7999,7199);//10Khz的计数频率，计数到8000为800ms  
	uart_init(RATE_BOUND);
	uart2_init(RATE_BOUND);
	uart3_init(4800);	 //串口初始化为4800
	LED_Init();
 
	while(1){
	//	logi("8266-------");
		if(USART_RX_STA & BIT_AT_END) { //At command
			len = getBufferLen();
			readAtFromBuffer(at_buffer, len);
			resetBuffer(); //reset buffer when received at
			
			//logAt(at_buffer, len);
			esp8266_at_parse((char*)at_buffer);
			
			USART_RX_STA &= ~BIT_AT_END;
		} else { //tcp package
			try2ParseTcpPacket();
			if(USART_RX_STA & BIT_TCP_PACKET) {
				no_conn_time_out = 0; //reset
				parsePacket();
				USART_RX_STA &= ~BIT_TCP_PACKET;
				resetBuffer();
			}
		}
		
		if(no_conn_time_out >= 100){
			logi("8266 reboot...");
			no_conn_time_out = 0;
			//logi("no_conn_time_out: %d", no_conn_time_out);
			if(ESP8266_STA == ESP8266_TCP_DISCONNECTED){
				ESP8266_STA = ESP8266_WIFI_DISCONNECTED;
				UART_send_str(USART_8266, "+++");
				delay_ms(1000);
			}
			UART_send_str(USART_8266, "AT+RST\n");
		}
		
	//	if(++times % 100==0){
			//logi("debug uart len: %d", getBufferLen());
//			LED0 = !LED0;
//		}
		
		delay_ms(1);
		check_buffer();      //接受缓冲循环数据处理
		//1---巡查下位机数据		
		if (Send_Flag){	    //定时巡查		
			while(Wire24G_Data[Scan_Cnt].Wire24G_Addrees == 0){  //直到有2.4G无线模块
			Scan_Cnt++;
			if(Scan_Cnt==Wire24G_Max)	Scan_Cnt=0;
			}
			if(Wire24G_Data[Scan_Cnt].Wire24G_Addrees > 0){  //有2.4G无线地址
					Tx_date_Buffer[2] = Wire24G_Data[Scan_Cnt].Wire24G_Addrees;  //设置目标地址
					Tx_date_Buffer[3] = Main_Address;                             //主机地址
					Tx_date_Buffer[4] = Wire24G_Data[Scan_Cnt].Data_value1;       //下发数据
					Tx_date_Buffer[5] = Wire24G_Data[Scan_Cnt].Data_value2;  
					Tx_date_Buffer[6] = Wire24G_Data[Scan_Cnt].Data_value3;
					Tx_date_Buffer[2] = Tx_date_Buffer[2]|0xc0;               //设置命令：查询下位机数据
			}				
			Sum = 0;           
			for(Temp=0;Temp<7;Temp++)
				 Sum += Tx_date_Buffer[Temp];        //计算校验和
			Tx_date_Buffer[7] = Sum;             //发送数据的校验和
			UART_Send(USART3,Tx_date_Buffer,8);  //串口3发送命令
		//		UART_Send(USART1,Tx_date_Buffer,8);  //串口1发送命令
			Scan_Cnt++;         //下一条巡查
			if(Scan_Cnt==Wire24G_Max)	Scan_Cnt=0;	
			Send_Flag = 0; //巡查完成
   }
	}
	
}

/**
* add one device status to json object.
* @param root the json root.
* @param mac433 the 433 mac address.
* @param light_no the light number under 433.
* @param action light status.
* @return the root of the json.
*/
cJSON* addDeviceStatus2Json(cJSON *root, uint32_t mac433, uint32_t light_no, uint32_t action){
	cJSON* fmt = cJSON_CreateObject();
	char temp[6]={0};
	sprintf(temp, "%02d-%02d", mac433, light_no);
	cJSON_AddNumberToObject(fmt, "a", action);
	cJSON_AddItemToObject(root, temp, fmt);
	return root;
}

//processing device sync ping command.
void prosDeviceSyncPing() {
	char* temp;
	cJSON *root = cJSON_CreateObject();

	addDeviceStatus2Json(root, 0, 0, 1);
	addDeviceStatus2Json(root, 0, 1, 0);

	temp = cJSON_PrintUnformatted(root);
	logi("%s", temp);
	newPacketSend(CMD_ROOM_DEVICE_SYNC_PONG, temp);
	Send_Packet_And_Free();
	free(temp);
	cJSON_Delete(root);
}

/***processing server command**/
void parsePacket(){
	char m[8] = {0};
	u8 Sum,Temp;
	uint32_t result;
	uint32_t type;
	uint32_t action;
	uint32_t mac433;
	uint32_t light_no;
	logi("recv cmd: 0x%04x\r\n", packetRecv.cmd);
	switch(packetRecv.cmd){
		case CMD_PING:
			newPacketSendNullData(CMD_PONG);
			Send_Packet_And_Free();
			break;
		case CMD_ROOM_LOGIN_RSP:
		  result = readIntOff(packetRecv.data, 0);
			logi("login result: %s\r\n", result == 0 ? "success" : "fail" );
			if(result == 0) {
				ESP8266_STA = ESP8266_TCP_DISCONNECTED;
			}
			break;
		case CMD_DEVICE_PUSH:
			type = readShortOff(packetRecv.data, 0);
			action = readIntOff(packetRecv.data, 2);
			
			memcpy(packetRecv.data, m, packetRecv.bodyLength - 6);
			mac433 = atoi(strtok(m, "-"));
			light_no = atoi(strtok(NULL, "-"));
			logi("type: %d, action: %d, mac433: %d, lightNo: %d, m: %s", type, action, mac433, light_no, m);

			Tx_date_Buffer[2]= (u8)(mac433 &0xFF);     //2.4G无线模块地址
			Tx_date_Buffer[2] = Tx_date_Buffer[2]|0x40;   //设置RGB
			Tx_date_Buffer[3]= Main_Address;
			Tx_date_Buffer[4] =(u8)(action>>8 & 0xFF);
			Tx_date_Buffer[5] =(u8)(action>>16 & 0xFF);
			Tx_date_Buffer[6] =(u8)(action & 0xFF);
		
			Sum = 0;           
		  for(Temp=0;Temp<7;Temp++)
			 Sum += Tx_date_Buffer[Temp];        //计算校验和
		  Tx_date_Buffer[7] = Sum;             //发送数据的校验和
		  UART_Send3(USART3,Tx_date_Buffer,8);  //串口3发送命令控制命令

			break;
		case CMD_ROOM_DEVICE_SYNC_PING:
			prosDeviceSyncPing();
			break;
		default:
			break;
	}
	freePacket(&packetRecv);
}
