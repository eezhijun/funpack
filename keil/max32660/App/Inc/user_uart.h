#ifndef  _USER_UART_H_
#define _USER_UART_H_

#include "uart.h"


void uart_init(void);
void print_data(uint8_t *buf, uint8_t size);
void read_cb(uart_req_t* req, int error);
#endif

