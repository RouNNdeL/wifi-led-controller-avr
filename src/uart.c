//
// Created by Krzysiek on 2019-07-30.
//

#include "uart.h"
#include <avr/io.h>

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
    for(uint16_t i = 0; i < length; ++i) {
        /* Wait for UDR0 data register to be empty */
        while(!(UCSR0A & (1 << UDRE0)));
        UDR0 = ptr[i];
    }
}

void uart_send(uint8_t data) {
    /* Wait for UDR0 data register to be empty */
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}