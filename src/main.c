//
// Created by Krzysiek on 2019-07-30.
//

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <effects.h>
#include <string.h>
#include "uart.h"
#include "memory.h"
#include "config.h"

extern void output_grb_pc2(uint8_t *ptr, uint16_t count);

volatile uint32_t frame;
volatile uint8_t pos;

device_effect device_profiles[DEVICE_COUNT];
global_settings globals_addr;

#define MAX_LEDS 13

int main() {
    /* Setup ports */
    DDRC = 0x04;
    DDRB = 0x20;
    DDRD = 0x1c;

    /* Global interrupt enable */
    sei();

    /* Setup UART */
    init_uart();

    uint8_t leds[LED_COUNT * 3];
    memset(leds, 0, sizeof(leds));

    for(;;) {
        uint8_t args[6];
        args[ARG_BIT_PACK] = RAINBOW_SIMPLE;
        args[ARG_RAINBOW_BRIGHTNESS] = 255;

        uint16_t times[6] = {0};
        times[TIME_FADEOUT] = 500;

        memset(leds, 0, MAX_LEDS * 3);
        digital_effect(RAINBOW, leds + pos * 3, 1, 0, frame, times, args, 0, 0);

        output_grb_pc2(leds, MAX_LEDS * 3);

        frame++;

        _delay_ms(10);
    }
}

/* UART RX interrupt */
ISR(USART_RX_vect) {
    uint8_t val = UDR0;

    if(val == 'r') {
        if(pos < MAX_LEDS - 1) {
            pos++;
        }
    }

    if(val == 'l') {
        if(pos > 0) {
            pos--;
        }
    }

    uart_send(pos);
}
