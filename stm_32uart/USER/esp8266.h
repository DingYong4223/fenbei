#ifndef __ESP8266_H
#define __ESP8266_H
#include "sys.h"

//state for 8266
#define ESP8266_WIFI_DISCONNECTED         0
#define ESP8266_WIFI_CONNECTING           1
#define ESP8266_WIFI_CONNECTED            2
#define ESP8266_TCP_DISCONNECTED          3
#define ESP8266_TCP_CONNECTED             4

#define AT_COMMAND_DELAY               500
#define ESP8266_MAC_LAN                18

//TCP
//#define ESP8266_TCP_SERVER             "192.168.0.103"
#define ESP8266_TCP_SERVER             "106.55.12.110"
#define ESP8266_TCP_PORT               "8888"

void esp8266_tcp_send(uint8_t* bytes, uint16_t len);
void esp8266_at_parse(char* command);
uint16_t get_AT_Command_2_AtBuffer(void);
void esp8266_send_at_command(char* command);
void get8266mac(void);

#endif
