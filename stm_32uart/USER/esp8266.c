#include "esp8266.h"
#include "usart.h"
#include "delay.h"
#include "string.h"
#include "packet.h"
#include "msg.h"
#include "round_queue.h"
#include "func.h"
#include <stdlib.h>

u16 ESP8266_STA=ESP8266_WIFI_DISCONNECTED;
extern uint16_t TCP_CMD_TO_SEND;
extern Packet packetRecv;
extern Packet packetSend;
uint16_t at_len;
uint8_t at_buffer[USART_REC_LEN];
u16 temp;
char at_command[40]={0};
char mac[ESP8266_MAC_LAN]={0};

void esp8266_tcp_send(uint8_t* bytes, uint16_t len){
	UART_send_str(USART_8266, "AT+CIPSEND\n");
	UART_Send(USART_8266, bytes, len);
}

uint16_t get_AT_Command_2_AtBuffer(){
	if(USART_RX_STA & BIT_AT_END) { //At command
		at_len = getBufferLen();
		readAtFromBuffer(at_buffer, at_len);
		USART_RX_STA &= ~BIT_AT_END;
		return at_len;
	}
	return 0;
}

void esp8266_send_at_command(char* command){
	delay_ms(AT_COMMAND_DELAY);
	UART_send_str(USART_8266, command);
	delay_ms(AT_COMMAND_DELAY);
	while(0 == get_AT_Command_2_AtBuffer()) {
		delay_ms(AT_COMMAND_DELAY);
	}
}

void get8266mac(){
	char* macIndex;
	char* macStart;
	char* macEnd;
	esp8266_send_at_command("AT+CIFSR\r\n");
	//char* input="+CIFSR:APIP,\"192.168.4.1\"\r\n+CIFSR:APMAC,\"da:f1:5b:0f:f0:c1\"\r\n+CIFSR:STAIP,\"192.168.0.105\"\r\n+CIFSR:STAMAC,\"d8:f1:5b:0f:f0:c1\"\r\n\r\nOK";
	while(strstr((char *)at_buffer, "CIFSR:STAMAC") == NULL) {
		esp8266_send_at_command("AT+CIFSR\r\n");
	}
	macIndex = strstr((char*)at_buffer, "CIFSR:STAMAC");
	macStart = strstr(macIndex, "\"");
	macEnd= strstr(++macStart, "\"");
	strncpy(mac, macStart, macEnd - macStart);
	
	mac[ESP8266_MAC_LAN - 1] = 0;
	del_char(mac, ':');
	logi("get mac:%s", mac);
}

//parse at command which end by 0x0d,0x0a
//WIFI DISCONNECT
//WIFI CONNECTED
//WIFI GOT IP
void esp8266_at_parse(char* command){
	uint16_t hasDefWifi = 0;
	uint16_t issetting = 0;
	logi("at command: %s", command);
	if(strstr(command, "FI DISCONNECT") != NULL) {
		ESP8266_STA=ESP8266_WIFI_DISCONNECTED;
	} else if(strstr(command, "GOT IP") != NULL) {
		delay_ms(AT_COMMAND_DELAY);
		UART_send_str(USART_8266, "AT+CWLAP\n");
		delay_ms(AT_COMMAND_DELAY);
		while(0 == get_AT_Command_2_AtBuffer()) {
			delay_ms(AT_COMMAND_DELAY);
		}
		while(strstr((char *)at_buffer, "\r\n\r\nOK") == NULL) {
			logi("temp: %s", (char*)at_buffer);
			delay_ms(2 * AT_COMMAND_DELAY);
			while(0 == get_AT_Command_2_AtBuffer()) {
				delay_ms(AT_COMMAND_DELAY);
			}
		}
		logi("--------------");
		logi((char*)at_buffer);
		if(strstr((char *)at_buffer, "uFi_0462B1") != NULL) {
			logi("debugger wifi detected...");
		} else {
			logi("no debugger wifi detected...");
		}
		// if(hasDefWifi){
			
		// } else if(issetting){
		// 	conn2server_and_login();
		// }
	}
}
/**
* start to connect to server. and login after.
*/
void conn2server_and_login(){
		ESP8266_STA=ESP8266_WIFI_CONNECTED;
		get8266mac();
		sprintf(at_command, "AT+CIPSTART=\"TCP\",\"%s\",%s\n", ESP8266_TCP_SERVER, ESP8266_TCP_PORT);
		esp8266_send_at_command(at_command);
		while(strstr((char *)at_buffer, "CONNECT\r\n\r\nOK") == NULL
										&& strstr((char *)at_buffer, "CONNECTED") == NULL) {
			esp8266_send_at_command(at_command);
			logi((char*)at_buffer);
			if(strstr((char *)at_buffer, "DISCONNECT") != NULL) {
				ESP8266_STA=ESP8266_WIFI_DISCONNECTED;
				return;
			}
			delay_ms(1000);
		}
		logi("tcp connected!");
		
		esp8266_send_at_command("AT+CIPMODE=1\n");
		logi((char*)at_buffer);
		
		esp8266_send_at_command("AT+CIPSEND\n");
		logi((char*)at_buffer);
		
		logi("login packet send...");
		newPacketSend(CMD_ROOM_LOGIN_REQ, mac);
		Send_Packet_And_Free();
}
