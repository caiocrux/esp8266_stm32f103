#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "esp8266.h"
#include "esp8266_ll.h"
#include "circularbuffer.h"
#include "util.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t data;
int8_t Response_Status;
volatile int16_t Counter = 0, pointer = 0;
uint32_t TimeOut = 0;
char RESPONSE_BUFFER[DEFAULT_BUFFER_SIZE];

void Read_Response(char* _Expected_Response)
{
  uint8_t EXPECTED_RESPONSE_LENGTH = strlen(_Expected_Response);
  uint32_t TimeCount = 0, ResponseBufferLength;
  char RECEIVED_CRLF_BUF[EXPECTED_RESPONSE_LENGTH];

  while(1)
  {
    if(TimeCount >= (DEFAULT_TIMEOUT+TimeOut))
    {
	TimeOut = 0;
	Response_Status = ESP8266_RESPONSE_TIMEOUT;
	return;
    }

    if(Response_Status == ESP8266_RESPONSE_STARTING)
    {
	Response_Status = ESP8266_RESPONSE_WAITING;
    }

    ResponseBufferLength = strlen(RESPONSE_BUFFER);
    if (ResponseBufferLength)
    {
    HAL_Delay(1);
	TimeCount++;
	if (ResponseBufferLength==strlen(RESPONSE_BUFFER))
	{
	    for (uint16_t i=0;i<ResponseBufferLength;i++)
	    {
		memmove(RECEIVED_CRLF_BUF, RECEIVED_CRLF_BUF + 1, EXPECTED_RESPONSE_LENGTH-1);
		RECEIVED_CRLF_BUF[EXPECTED_RESPONSE_LENGTH-1] = RESPONSE_BUFFER[i];
		if(!strncmp(RECEIVED_CRLF_BUF, _Expected_Response, EXPECTED_RESPONSE_LENGTH))
		{
			TimeOut = 0;
			Response_Status = ESP8266_RESPONSE_FINISHED;
			return;
		}
	    }
	}
    }
    HAL_Delay(1);
    TimeCount++;
  }
}

void ESP8266_Clear(void)
{
	memset(RESPONSE_BUFFER,0,DEFAULT_BUFFER_SIZE);
	Counter = 0;	pointer = 0;
}

void Start_Read_Response(char* _ExpectedResponse)
{
	Response_Status = ESP8266_RESPONSE_STARTING;
	do {
		Read_Response(_ExpectedResponse);
	} while(Response_Status == ESP8266_RESPONSE_WAITING);

}

void GetResponseBody(char* Response, uint16_t ResponseLength)
{

	uint16_t i = 12;
	char buffer[5];
	while(Response[i] != '\r')
	++i;

	strncpy(buffer, Response + 12, (i - 12));
	ResponseLength = atoi(buffer);

	i += 2;
	uint16_t tmp = strlen(Response) - i;
	memcpy(Response, Response + i, tmp);

	if(!strncmp(Response + tmp - 6, "\r\nOK\r\n", 6))
	memset(Response + tmp - 6, 0, i + 6);
}

bool WaitForExpectedResponse(char* ExpectedResponse)
{
	HAL_UART_Receive_IT(&huart2,&data,1);
	Start_Read_Response(ExpectedResponse);	/* First read response */
	if((Response_Status != ESP8266_RESPONSE_TIMEOUT)){
		return true;				/* Return true for success */
	}
	return false;				/* Else return false */
}

bool SendATandExpectResponse(char* ATCommand, char* ExpectedResponse)
{
	ESP8266_Clear();
	_esp8266_print(ATCommand);		/* Send AT command to ESP8266 */
	_esp8266_print("\r\n");
	return WaitForExpectedResponse(ExpectedResponse);
}

bool ESP8266_ApplicationMode(uint8_t Mode)
{
	char _atCommand[20];
	memset(_atCommand, 0, 20);
	sprintf(_atCommand, "AT+CIPMODE=%d", Mode);
	_atCommand[19] = 0;
	return SendATandExpectResponse(_atCommand, "\r\nOK\r\n");
}

bool ESP8266_ConnectionMode(uint8_t Mode)
{
	char _atCommand[20];
	memset(_atCommand, 0, 20);
	sprintf(_atCommand, "AT+CIPMUX=%d", Mode);
	_atCommand[19] = 0;
	return SendATandExpectResponse(_atCommand, "\r\nOK\r\n");
}

bool ESP8266_Begin()
{
	for (uint8_t i=0;i<5;i++) {
		//if(SendATandExpectResponse("ATE0","\r\nOK\r\n")||SendATandExpectResponse("AT","\r\nOK\r\n"))
		if(SendATandExpectResponse("ATE0","\r\nOK\r\n")){
			return true;
		}
	}
	return false;
}

bool ESP8266_Close()
{
	return SendATandExpectResponse("AT+CIPCLOSE","\r\nOK\r\n");
}

bool ESP8266_WIFIMode(uint8_t _mode)
{
	char _atCommand[20];
	memset(_atCommand, 0, 20);
	sprintf(_atCommand, "AT+CWMODE=%d", _mode);
	_atCommand[19] = 0;
	return SendATandExpectResponse(_atCommand, "\r\nOK\r\n");
}

uint8_t ESP8266_JoinAccessPoint(char* _SSID, char* _PASSWORD)
{
	char _atCommand[60];
	memset(_atCommand, 0, 60);
	sprintf(_atCommand, "AT+CWJAP=\"%s\",\"%s\"", _SSID, _PASSWORD);
	_atCommand[59] = 0;
	if(SendATandExpectResponse(_atCommand, "\r\nWIFI CONNECTED\r\n"))
	return ESP8266_WIFI_CONNECTED;
	else{
		if(strstr(RESPONSE_BUFFER, "+CWJAP:1"))
		return ESP8266_CONNECTION_TIMEOUT;
		else if(strstr(RESPONSE_BUFFER, "+CWJAP:2"))
		return ESP8266_WRONG_PASSWORD;
		else if(strstr(RESPONSE_BUFFER, "+CWJAP:3"))
		return ESP8266_NOT_FOUND_TARGET_AP;
		else if(strstr(RESPONSE_BUFFER, "+CWJAP:4"))
		return ESP8266_CONNECTION_FAILED;
		else
		return ESP8266_JOIN_UNKNOWN_ERROR;
	}
}

uint8_t ESP8266_connected()
{
	SendATandExpectResponse("AT+CIPSTATUS", "\r\nOK\r\n");
	if(strstr(RESPONSE_BUFFER, "STATUS:2"))
	return ESP8266_CONNECTED_TO_AP;
	else if(strstr(RESPONSE_BUFFER, "STATUS:3"))
	return ESP8266_CREATED_TRANSMISSION;
	else if(strstr(RESPONSE_BUFFER, "STATUS:4"))
	return ESP8266_TRANSMISSION_DISCONNECTED;
	else if(strstr(RESPONSE_BUFFER, "STATUS:5"))
	return ESP8266_NOT_CONNECTED_TO_AP;
	else
	return ESP8266_CONNECT_UNKNOWN_ERROR;
}

uint8_t ESP8266_Conect_To_Server(uint8_t _ConnectionNumber, char* Domain, char* Port, uint8_t Keep_Alive) {
	bool _startResponse = false;
	char _atCommand[60];
	memset(_atCommand, 0, 60);
	_atCommand[59] = 0;

	if(SendATandExpectResponse("AT+CIPMUX?", "CIPMUX:0")) {
		sprintf(_atCommand, "AT+CIPSTART=\"TCP\",\"%s\",%s,%d", Domain, Port, Keep_Alive);
	}
	else {
		sprintf(_atCommand, "AT+CIPSTART=\"%d\",\"TCP\",\"%s\",%s", _ConnectionNumber, Domain, Port);
	}
	_startResponse = SendATandExpectResponse(_atCommand, "CONNECT\r\n");
	if(!_startResponse)
	{
		if(Response_Status == ESP8266_RESPONSE_TIMEOUT){
			return ESP8266_RESPONSE_TIMEOUT;
		}
		return ESP8266_RESPONSE_ERROR;
	}
	return ESP8266_RESPONSE_FINISHED;
}

uint8_t ESP8266_Send(char* Data) {
	char _atCommand[20];
	memset(_atCommand, 0, 20);
	sprintf(_atCommand, "AT+CIPSEND=%d", (strlen(Data)+2));
	_atCommand[19] = 0;

	if ( SendATandExpectResponse(_atCommand, "\r\nOK\r\n>")){
		if(!SendATandExpectResponse(Data, "\r\nSEND OK\r\n")){
			if(Response_Status == ESP8266_RESPONSE_TIMEOUT){
				return ESP8266_RESPONSE_TIMEOUT;
			}
			return ESP8266_RESPONSE_ERROR;
		}
		return ESP8266_RESPONSE_FINISHED;
	}
	else {
		return ESP8266_RESPONSE_SERVER_DISCONNECTED;
	}
}

int16_t ESP8266_DataAvailable(void)
{
	return (Counter - pointer);
}

uint8_t ESP8266_DataRead(void)
{
	if(pointer < Counter)
	return RESPONSE_BUFFER[pointer++];
	else{
		ESP8266_Clear();
		return 0;
	}
}

uint16_t Read_Data(char* _buffer)
{
	uint16_t len = 0;
	while(ESP8266_DataAvailable() > 0)
	_buffer[len++] = ESP8266_DataRead();
	return len;
}
bool ESP8266_Paser_Received_Data(char* buffer,char *message_parser) {
	int pattern = 0;
	int string = 0;
	pattern = match(buffer,":");
	if (pattern == -1) {
		return false;
	}
	pattern = pattern+1; //avoid the :
	while(buffer[pattern] != '\r') {
		message_parser[string] = buffer[pattern];
		string++;
		pattern++;
	}
	return true;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	HAL_UART_Receive_IT(&huart2,&data,1);
	RESPONSE_BUFFER[Counter] = data;
	Counter++;
	if(Counter == DEFAULT_BUFFER_SIZE){
		Counter = 0; pointer = 0;
	}
}


