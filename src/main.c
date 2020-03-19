//
// Created by Krzysiek on 2019-07-30.
//

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <effects.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "uart.h"
#include "memory.h"

extern void output_grb_pc2(uint8_t *ptr, uint16_t count);

volatile uint8_t uart_receive_size;
volatile uint8_t uart_state;
uint8_t *uart_data;
uint8_t uart_cmd;
uint8_t uart_invalid_count;

volatile uint8_t new_frame;
uint32_t frame;
uint32_t reboot_frame = UINT32_MAX;

uint8_t virtual_led_count[DEVICE_COUNT] = VIRTUAL_DEVICES;
device_settings settings[DEVICE_COUNT] = {0};

void reboot() {
    cli();
    wdt_enable(WDTO_15MS);
    PORTD |= (1 << 3);
    for (;;);
}

uint8_t get_expected_message_size(uint8_t command) {
    switch (command) {
        case CMD_SIMPLE_DATA:
            return 6;
        case CMD_REBOOT:
            return 0;
        default:
            return 0;
    }
}

uint8_t valid_command(uint8_t command) {
    return command == CMD_SIMPLE_DATA || command == CMD_REBOOT;
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
        case CMD_REBOOT:
            reboot_frame = frame + FPS / 10;
    }

    return false;
}

int main() {
    /* Setup ports */
    DDRC = 0x04;
    DDRB = 0x20;
    DDRD = 0x1e;

    OCR1A = 31250;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11);
    TIMSK1 |= (1 << OCIE1A);

    /* Global interrupt enable */
    sei();
    wdt_enable(WDTO_60MS);

    /* Setup UART */
    init_uart();

    uart_data = NULL;
    new_frame = 1;
    uart_invalid_count = 0;

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

    for (;;) {
        if (uart_invalid_count > 100) {
            reboot();
        }

        if (!uart_buffer_is_empty()) {
            switch (uart_state) {
                case STATE_NONE: {
                    uint8_t val = uart_buffer_poll();
                    if (val == UART_BEGIN) {
                        uart_invalid_count = 0;
                        uart_state = STATE_RECEIVE_COMMAND;
                    } else {
                        uart_send(UART_INVALID_SEQUENCE);
                        uart_invalid_count++;
                    }
                    break;
                }
                case STATE_RECEIVE_COMMAND:
                    uart_cmd = uart_buffer_poll();
                    if (valid_command(uart_cmd)) {
                        uart_state = STATE_RECEIVE_LENGTH;
                    } else {
                        uart_state = STATE_NONE;
                        uart_send(UART_INVALID_COMMAND);
                    }
                    break;
                case STATE_RECEIVE_LENGTH:
                    uart_receive_size = uart_buffer_poll();

                    if (uart_receive_size != get_expected_message_size(uart_cmd)) {
                        uart_send(UART_INVALID_LENGTH);
                        uart_state = STATE_NONE;
                    } else if(uart_receive_size > 0){
                        uart_state = STATE_RECEIVE_DATA;
                    } else {
                        uart_state = STATE_RECEIVE_DONE;
                    }

                    break;
                case STATE_RECEIVE_DATA:
                    if (uart_buffer_size() < uart_receive_size) {
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
                        if (uart_data != NULL || uart_receive_size == 0) {
                            if (handle_data(uart_cmd, uart_data)) {
                                uart_send(UART_ACK);
                            }
                        }

                        uart_cmd = 0;
                        uart_data = NULL;
                    } else {
                        uart_send(UART_INVALID_SEQUENCE);
                    }

                    uart_state = STATE_NONE;
                    break;
                default:
                    uart_state = STATE_NONE;
            }
        }

        if (new_frame) {
            new_frame = 0;
            frame++;

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
            wdt_reset();

            if(frame >= reboot_frame) {
                reboot();
            }
        }
    }
}

ISR(USART_RX_vect) {
    uint8_t val = UDR0;
    if (!uart_buffer_add(val)) {
        uart_send(UART_BUFFER_OVERFLOW);
        uart_buffer_reset();
    }
}

ISR(TIMER1_COMPA_vect) {
    new_frame = 1;
}
