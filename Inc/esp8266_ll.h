/*
 * esp8266_ll.h
 *
 *  Created on: 15 de mai de 2018
 *      Author: tpv
 */

#ifndef ESP8266_LL_H_
#define ESP8266_LL_H_
#include <stdbool.h>
void esp8266_set_power_down_hw(bool on_off);
void esp8266_reset_hw(void);
void _esp8266_print(char *ptr);
void _esp8266_putch(uint8_t message);

#endif /* ESP8266_LL_H_ */
