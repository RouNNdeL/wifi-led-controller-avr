//
// Created by Krzysiek on 2019-07-30.
//

#ifndef AVR_UART_H
#define AVR_UART_H

#include <stdint.h>

#define BAUD 9600                                   // define baud
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)            // set baud rate value for UBRR

void init_uart();

void uart_send_bytes(const uint8_t *ptr, uint16_t length);

void uart_send(uint8_t data);

#endif //AVR_UART_H
