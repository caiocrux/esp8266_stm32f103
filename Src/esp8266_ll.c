/*
 * esp8266_ll.c
 *
 *  Created on: 15 de mai de 2018
 *      Author: tpv
 */
#include <stdbool.h>
#include "esp8266_ll.h"
#include "stm32f1xx_hal.h"
#include "usart.h"

void esp8266_set_power_down_hw(bool on_off){
	if(on_off){
		HAL_GPIO_WritePin(WIFI_CH_PD_GPIO_Port,WIFI_CH_PD_Pin,GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(WIFI_CH_PD_GPIO_Port,WIFI_CH_PD_Pin,GPIO_PIN_SET);
		HAL_Delay(1000);
	}

}

void esp8266_reset_hw(void){
	HAL_GPIO_WritePin(WIFI_RST_GPIO_Port,WIFI_RST_Pin,GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(WIFI_RST_GPIO_Port,WIFI_RST_Pin,GPIO_PIN_SET);
}

void _esp8266_print(char *ptr){
	while (*ptr != 0) {
		_esp8266_putch(*ptr++);
	}
}

void _esp8266_putch(char message){
	HAL_UART_Transmit(&huart2,&message,sizeof(char),100);
}
