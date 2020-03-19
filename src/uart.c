//
// Created by Krzysiek on 2019-07-30.
//

#include <avr/io.h>
#include "uart.h"
#include "memory.h"
#include <stdbool.h>

volatile uint8_t uart_buffer[UART_BUFFER_SIZE];
volatile uart_buffer_t front = 0;
volatile int32_t rear = -1;
volatile uart_buffer_t itemCount = 0;

bool uart_buffer_is_empty() {
    return itemCount == 0;
}

bool is_full() {
    return itemCount == UART_BUFFER_SIZE;
}

void uart_buffer_reset() {
    front = 0;
    rear = -1;
    itemCount = 0;
}

uint8_t uart_buffer_size() {
    return itemCount;
}

bool uart_buffer_add(uint8_t data) {
    if (itemCount < UART_BUFFER_SIZE) {
        if (rear == UART_BUFFER_SIZE - 1) {
            rear = -1;
        }

        uart_buffer[++rear] = data;
        itemCount++;
        return true;
    }
    return false;
}

uint8_t uart_buffer_poll() {
    uint8_t data = uart_buffer[front++];

    if (front == UART_BUFFER_SIZE) {
        front = 0;
    }

    itemCount--;
    return data;
}

void init_uart() {
    /* Set LOW and HIGH bytes of BAUD */
    UBRR0H = (BAUDRATE >> 8);
    UBRR0L = BAUDRATE;

    /* Enable RX and TX, and RX interrupt */
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);

    /* Set 8 data bits, 1 stop bit, no parity */
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
}

void uart_send_bytes(const uint8_t *ptr, uint16_t length) {
    for (uint16_t i = 0; i < length; ++i) {
        /* Wait for UDR0 data register to be empty */
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = ptr[i];
    }
}

void uart_send(uint8_t data) {
    /* Wait for UDR0 data register to be empty */
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}
