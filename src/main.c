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

device_effect device_profiles[DEVICE_COUNT];
global_settings globals_addr;

#define BRIGHTNESS 255

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

        uint8_t colors[9];
        set_color_grb(colors, 0, COLOR_RED);
        set_color_grb(colors, 1, COLOR_GREEN);
        set_color_grb(colors, 2, COLOR_BLUE);

        digital_effect(RAINBOW, leds, LED_COUNT, 0, frame, times, args, colors, 3);

        output_grb_pc2(leds, sizeof(leds));

        frame++;

        _delay_ms(10);
    }
}

/* UART RX interrupt */
ISR(USART_RX_vect) {
    uint8_t val = UDR0;
}
