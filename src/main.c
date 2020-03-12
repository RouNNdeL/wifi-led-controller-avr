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
#include <stdlib.h>
#include <stdbool.h>
#include "uart.h"
#include "memory.h"

extern void output_grb_pc2(uint8_t *ptr, uint16_t count);

volatile uint8_t uart_receive_size;
volatile uint8_t uart_state;
uint8_t *uart_data = NULL;
uint8_t uart_cmd;

uint8_t virtual_led_count[DEVICE_COUNT] = VIRTUAL_DEVICES;
device_settings settings[DEVICE_COUNT] = {0};

void debug(uint8_t num) {
    PORTD |= (1 << 3);
    while (num--) {
        _delay_us(10);
    }
    PORTD &= ~(1 << 3);
}

uint8_t get_expected_message_size(uint8_t command) {
    switch (command) {
        case CMD_SIMPLE_DATA:
            return 6;
        default:
            return 0;
    }
}

bool handle_data(uint8_t cmd, uint8_t *data) {
    switch (cmd) {
        case CMD_SIMPLE_DATA: {
            uint8_t device_index = data[0];
            if (device_index < DEVICE_COUNT) {
                memcpy(&settings[device_index], data + 1, sizeof(device_settings));
                return true;
            } else {
                uart_send(UART_DEVICE_INDEX_OUT_OF_BOUNDS);
            }
            break;
        }
    }

    return false;
}

int main() {
    /* Setup ports */
    DDRC = 0x04;
    DDRB = 0x20;
    DDRD = 0x1e;

    /* Global interrupt enable */
    sei();

    /* Setup UART */
    init_uart();

    uart_buffer_add(12);

    uint8_t led_buffer[LED_COUNT * 3];
    memset(led_buffer, 0, LED_COUNT * 3);
    settings[0].flags = 1;
    settings[0].brightness = 50;
    set_color_manual_grb(settings[0].color, COLOR_RED);
    settings[1].flags = 1;
    settings[1].brightness = 255;
    set_color_manual_grb(settings[1].color, COLOR_GREEN);
    settings[2].flags = 1;
    settings[2].brightness = 255;
    set_color_manual_grb(settings[2].color, COLOR_BLUE);
    output_grb_pc2(led_buffer, sizeof(led_buffer));

    for (;;) {
        if (!uart_buffer_is_empty()) {
            switch (uart_state) {
                case STATE_NONE:
                    if (uart_buffer_poll() == UART_BEGIN) {
                        uart_state = STATE_RECEIVE_LENGTH;
                    } else {
                        uart_send(UART_INVALID_SEQUENCE);
                    }
                    break;
                case STATE_RECEIVE_LENGTH:
                    uart_receive_size = uart_buffer_poll();
                    uart_state = STATE_RECEIVE_DATA;
                    break;
                case STATE_RECEIVE_DATA:
                    if (uart_buffer_size() < uart_receive_size) {
                        break;
                    }

                    uart_cmd = uart_buffer_poll();
                    uart_receive_size--;

                    debug(uart_receive_size);
                    debug(get_expected_message_size(uart_cmd));

                    if (uart_receive_size != get_expected_message_size(uart_cmd)) {
                        uart_send(UART_INVALID_LENGTH);
                        uart_state = STATE_NONE;
                        break;
                    }

                    if (uart_receive_size > 0) {
                        uart_data = malloc((size_t) uart_receive_size);
                        for (uint8_t i = 0; i < uart_receive_size; ++i) {
                            uart_data[i] = uart_buffer_poll();
                        }
                    }

                    uart_state = STATE_RECEIVE_DONE;
                    break;
                case STATE_RECEIVE_DONE:
                    if (uart_buffer_poll() == UART_END) {
                        if (uart_data != NULL) {
                            if(handle_data(uart_cmd, uart_data)) {
                                uart_send(UART_ACK);
                            }
                        }

                        uart_cmd = 0;
                        uart_data = NULL;
                        uart_state = STATE_NONE;
                    } else {
                        uart_send(UART_INVALID_SEQUENCE);
                    }
                    break;
                default:
                    uart_state = STATE_NONE;
            }
        }

        uint8_t led_index = 0;
        for (uint8_t i = 0; i < DEVICE_COUNT; i++) {
            uint8_t led_count = virtual_led_count[i];
            uint8_t *index = led_buffer + led_index * 3;
            if (settings[i].flags & FLAG_ON) {
                set_all_colors(index,
                               color_brightness(settings[i].brightness, color_from_buf(settings[i].color)),
                               led_count, true);
            } else {
                memset(index, 0, led_count * 3);
            }

            led_index += led_count;
        }

        output_grb_pc2(led_buffer, sizeof(led_buffer));

        _delay_ms(1);
    }
}

ISR(USART_RX_vect) {
    uint8_t val = UDR0;

    debug(1);

    uart_buffer_add(val);
}
