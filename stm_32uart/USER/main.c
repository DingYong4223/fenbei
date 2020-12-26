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

extern u16 ESP8266_STA;
extern rcv_buffer_t rcv_buf;
extern Packet packetRecv;
extern Packet packetSend;
void parsePacket(void);
extern uint8_t at_buffer[];
extern char at_command[];
uint32_t no_conn_time_out=0;   //no connect time out.

 int main(void){
//	 char destin[20]={0};
//	 char* macEnd;
	u16 times=0, len;
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(RATE_BOUND);
	uart2_init(RATE_BOUND);
	LED_Init();
 
	while(1){
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
				parsePacket();
				USART_RX_STA &= ~BIT_TCP_PACKET;
			}
		}
		
		if(ESP8266_STA == ESP8266_TCP_DISCONNECTED && times % 100 == 0){
			no_conn_time_out += 1000;
			//logi("no_conn_time_out: %d", no_conn_time_out);
			if(no_conn_time_out >= 3 * TIME_PINGPONG_OUT){
				UART_send_str(USART_8266, "+++");
				delay_ms(1000);
				UART_send_str(USART_8266, "AT+RST\n");
				no_conn_time_out = 0;
				ESP8266_STA = ESP8266_WIFI_DISCONNECTED;
				logi("8266 reboot");
			}
		}
		
		if(++times % 100==0){
			//logi("debug uart len: %d", getBufferLen());
			LED0 = !LED0;
		}
		delay_ms(10);
	}

	// while(1){
	// 	len = get_AT_Command_2_AtBuffer();
	//  	if(len > 0) { //At command
	//  		logAt(at_buffer, len);
	//  	}
		
	// 	times++;
	// 	if(times % 300==0) {
	// 		get8266mac();
	// 	}
	// 	if(times % 100==0) {
	// 		LED0 = !LED0;
	// 		logi("this is delan's: %d", 100);
	// 		logi("-----------------------");
	// 	}
	// 	delay_ms(20);
	// }
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
	uint32_t result;
	uint32_t action;
	uint32_t mac433;
	uint32_t light_no;
	logi("recv cmd: 0x%04x\r\n", packetRecv.cmd);
	switch(packetRecv.cmd){
		case CMD_PING:
			no_conn_time_out = 0; //reset
			newPacketSendNullData(CMD_PONG);
			Send_Packet_And_Free();
			break;
		case CMD_ROOM_LOGIN_RSP:
		  result = readInt(packetRecv.data, 0);
			logi("login result: %s\r\n", result == 0 ? "success" : "fail" );
			if(result == 0) {
				ESP8266_STA = ESP8266_TCP_DISCONNECTED;
			}
			break;
		case CMD_DEVICE_PUSH:
			action = readInt(packetRecv.data, 0);
			logi("action: %d, light: %s", action, packetRecv.data + 4);
		
			mac433 = atoi(strtok((char*)(packetRecv.data + 4), "-"));
			light_no = atoi(strtok(NULL, "-"));
			logi("parse mac433: %d, light_no: %d", mac433, light_no);
			break;
		case CMD_ROOM_DEVICE_SYNC_PING:
			prosDeviceSyncPing();
			break;
		default:
			break;
	}
	freePacket(&packetRecv);
}


