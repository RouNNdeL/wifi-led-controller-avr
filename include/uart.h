//
// Created by Krzysiek on 2019-07-30.
//

#ifndef AVR_UART_H
#define AVR_UART_H

#include <stdint.h>
#include <stdbool.h>

#define BAUD 38400                                   // define baud
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)            // set baud rate value for UBRR

#define STATE_NONE 0x00
#define STATE_RECEIVE_COMMAND 0x02
#define STATE_RECEIVE_LENGTH 0x03
#define STATE_RECEIVE_DATA 0x04
#define STATE_RECEIVE_DONE 0x05

#define UART_BEGIN 0xB0
#define UART_END 0xB1
#define UART_ACK 0xA0
#define UART_FREE 0xA1
#define UART_BUSY 0xA2

#define CMD_SAVE_DEVICE 0x01
#define CMD_GET_DEVICE 0x02
#define CMD_GET_ALL_DEVICES 0x03
#define CMD_SET_COLOR 0x04
#define CMD_SET_BRIGHTNESS 0x05
#define CMD_SET_STATE 0x06
#define CMD_SET_EFFECT 0x07
#define CMD_SAVE_DEVICE_RESPONSE 0xA1
#define CMD_DEVICE_RESPONSE 0xA2
#define CMD_ALL_DEVICES_RESPONSE 0xA3
#define CMD_GET_TEMPS 0xB0
#define CMD_GET_TEMPS_RESPONSE 0xB1
#define CMD_GET_TEMPS_COUNT 0xB2
#define CMD_GET_TEMPS_COUNT_RESPONSE 0xB3
#define CMD_REBOOT 0xFF
#define CMD_REBOOT_RESPONSE 0xFF

#define UART_SUCCESS 0xBF

#define UART_BUFFER_OVERFLOW 0xE0
#define UART_INVALID_SEQUENCE 0xE1
#define UART_INVALID_LENGTH 0xE2
#define UART_DEVICE_INDEX_OUT_OF_BOUNDS 0xF1
#define UART_INVALID_COMMAND 0xF2

#define UART_LENGTH_INVALID_COMMAND 0xFF

void init_uart();

void uart_send_bytes(const uint8_t *ptr, uint16_t length);

void uart_send(uint8_t data);

bool uart_buffer_is_empty();

bool is_full();

uint8_t uart_buffer_size();

bool uart_buffer_add(uint8_t data);

uint8_t uart_buffer_peak();

uint8_t uart_buffer_poll();

void uart_buffer_reset();

#endif //AVR_UART_H
