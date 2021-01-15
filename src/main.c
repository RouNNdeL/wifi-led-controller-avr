//
// Created by Krzysiek on 2019-07-30.
//

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <effects.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <uart.h>
#include <memory.h>

extern void output_grb_pc2(uint8_t *ptr, uint16_t count);

volatile uint8_t uart_receive_size;
volatile uint8_t uart_state;
uint8_t *uart_data;
uint8_t uart_cmd;
uint8_t uart_invalid_count;

volatile uint8_t new_frame;
uint32_t frame;
uint32_t reboot_frame;
uint32_t save_frame;

uint8_t device_modified[DEVICE_COUNT] = {0};

led_count_t virtual_led_count[DEVICE_COUNT] = VIRTUAL_DEVICES;
device_settings settings[DEVICE_COUNT] = {0};

uint8_t led_buffer[LED_COUNT * 3];

/*
 * TODO: Save effects in the EEPROM, and load on demand.
 * We can introduce a config variable to compile to read effects from setup_effects and save them in the EEPROM.
 * Then compile normally and read the EEPROM when needed.
 */
device_effect device_effects[EFFECT_COUNT] = {0};

void reboot() {
    cli();
    wdt_enable(WDTO_15MS);
    PORTD |= (1 << 3);
    for (;;);
}

void send_device(uint8_t device_index) {
    uart_send(UART_BEGIN);
    uart_send(CMD_DEVICE_RESPONSE);
    uart_send(sizeof(device_settings) + 1);
    uart_send(device_index);
    uart_send_bytes((uint8_t *) &settings[device_index], sizeof(device_settings));
    uart_send(UART_END);
}

void on_save(uint8_t device_index) {
    save_frame = frame + SAVE_DELAY;
    device_modified[device_index] = 1;
    send_device(device_index);
}

uint8_t get_expected_message_size(uint8_t command) {
    switch (command) {
        case CMD_SAVE_DEVICE:
            return sizeof(device_settings) + 1;
        case CMD_GET_DEVICE:
            return 1;
        case CMD_SET_COLOR:
            return 4;
        case CMD_SET_BRIGHTNESS:
        case CMD_SET_STATE:
            return 2;
        case CMD_GET_ALL_DEVICES:
        case CMD_REBOOT:
        default:
            return 0;
    }
}

uint8_t valid_command(uint8_t c) {
    return c == CMD_SAVE_DEVICE || c == CMD_GET_DEVICE || c == CMD_GET_ALL_DEVICES || c == CMD_REBOOT ||
           c == CMD_SET_STATE || c == CMD_SET_BRIGHTNESS || c == CMD_SET_COLOR;
}

/**
 *
 * @param cmd command to process
 * @param data data buffer
 * @return non 0 if an ack response should be sent
 */
uint8_t handle_data(uint8_t cmd, uint8_t *data) {
    switch (cmd) {
        case CMD_SAVE_DEVICE: {
            uint8_t device_index = data[0];
            if (device_index < DEVICE_COUNT) {
                memcpy(&settings[device_index], data + 1, sizeof(device_settings));
                on_save(device_index);
                return UART_SUCCESS;
            } else {
                return UART_DEVICE_INDEX_OUT_OF_BOUNDS;
            }
        }
        case CMD_GET_DEVICE: {
            uint8_t device_index = data[0];
            if (device_index < DEVICE_COUNT) {
                send_device(device_index);
            } else {
                return UART_DEVICE_INDEX_OUT_OF_BOUNDS;
            }
            break;
        }
        case CMD_GET_ALL_DEVICES: {
            for (uint8_t i = 0; i < DEVICE_COUNT; ++i) {
                send_device(i);
            }
            break;
        }
        case CMD_SET_STATE: {
            uint8_t device_index = data[0];
            if (device_index < DEVICE_COUNT) {
                if (data[1]) {
                    settings[device_index].flags |= FLAG_ON;
                } else {
                    settings[device_index].flags &= ~FLAG_ON;
                }
                on_save(device_index);
                return UART_SUCCESS;
            } else {
                return UART_DEVICE_INDEX_OUT_OF_BOUNDS;
            }
        }
        case CMD_SET_BRIGHTNESS: {
            uint8_t device_index = data[0];
            if (device_index < DEVICE_COUNT) {
                settings[device_index].brightness = data[1];
                on_save(device_index);
                return UART_SUCCESS;
            } else {
                return UART_DEVICE_INDEX_OUT_OF_BOUNDS;
            }
        }
        case CMD_SET_COLOR: {
            uint8_t device_index = data[0];
            if (device_index < DEVICE_COUNT) {
                memcpy(&settings[device_index].color, data + 1, 3);
                on_save(device_index);
                return UART_SUCCESS;
            } else {
                return UART_DEVICE_INDEX_OUT_OF_BOUNDS;
            }
        }
        case CMD_REBOOT: {
            reboot_frame = frame + REBOOT_DELAY;
            return CMD_REBOOT_RESPONSE;
        }
    }

    return 0;
}

void handle_uart() {
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
            } else if (uart_receive_size > 0) {
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
                if (uart_data == NULL) {
                    reboot();
                }

                for (uint8_t i = 0; i < uart_receive_size; ++i) {
                    uart_data[i] = uart_buffer_poll();
                }
            }

            uart_state = STATE_RECEIVE_DONE;
            break;
        case STATE_RECEIVE_DONE:
            if (uart_buffer_poll() == UART_END) {
                if (uart_data != NULL || uart_receive_size == 0) {
                    uint8_t response_cmd = handle_data(uart_cmd, uart_data);
                    if (response_cmd) {
                        uart_send(UART_BEGIN);

                        uint8_t ack_buf[] = {response_cmd, 1, UART_ACK, UART_END};
                        uart_send_bytes(ack_buf, sizeof(ack_buf));
                    }
                }

                uart_cmd = 0;
                uart_data = NULL;
            } else {
                uart_send(UART_INVALID_SEQUENCE);
            }

            free(uart_data);
            uart_state = STATE_NONE;
            break;
        default:
            uart_state = STATE_NONE;
    }
}

uint16_t scale_timing(uint16_t time, uint8_t speed) {
    switch (speed) {
        case 0:
            return 0;
        case 1:
            return time * 16;
        case 2:
            return time * 12;
        case 3:
            return time * 8;
        case 4:
            return time * 4;
        case 5:
            return time * 2;
        case 6:
            return time * 1.5;
        case 7:
            return time;
        case 8:
            return time / 1.5;
        case 9:
            return time / 2;
        case 10:
            return time / 4;
        case 11:
            return time / 8;
        case 12:
            return time / 12;
        case 13:
            return time / 16;
        default:
            return time;
    }
}

void scale_timings(uint16_t src[TIME_COUNT], uint16_t dst[TIME_COUNT], uint8_t speed, uint8_t mask) {
    for (uint8_t i = 0; i < TIME_COUNT; ++i) {
        if (mask & (1 << i)) {
            dst[i] = scale_timing(src[i], speed);
        } else {
            dst[i] = src[i];
        }
    }
}

void output_effect(device_settings s, uint8_t *index, led_count_t led_count) {
    device_effect effect = device_effects[s.effect % EFFECT_COUNT];
    uint16_t timing[TIME_COUNT];
    scale_timings(effect.timing, timing, s.effect_speed, effect.flags);
    uint8_t *colors;
    uint8_t color_count; // Safety for effects with invalid configuration
    if (effect.flags & EFFECT_FLAG_INHERIT_COLOR) {
        colors = s.color;
        color_count = 1;
    } else {
        colors = effect.colors;
        color_count = effect.color_count;
    }
    digital_effect(effect.effect, index, led_count, 0, frame,
                   timing, effect.args, colors, color_count);

    for (uint8_t j = 0; j < led_count; ++j) {
        set_color_manual(index + j, color_brightness(s.brightness, color_from_buf(index + j)));
    }
}

void output_leds() {
    led_count_t led_index = 0;
    for (uint8_t i = 0; i < DEVICE_COUNT; ++i) {
        led_count_t led_count = virtual_led_count[i];
        uint8_t *index = led_buffer + led_index * 3;
        if (settings[i].flags & FLAG_ON) {
            if (settings[i].flags & FLAG_EFFECT_ON) {
                output_effect(settings[i], index, led_count);
            } else {
                set_all_colors(index, color_brightness(
                        settings[i].brightness, color_from_buf(settings[i].color)), led_count);
            }
        } else {
            memset(index, 0, led_count * 3);
        }

        if (settings[i].flags & FLAG_LOG_BRIGHTNESS) {
            for (uint8_t j = 0; j < led_count; ++j) {
                index[j] = actual_brightness(index[j]);
            }
        }

        led_index += led_count;
    }

    uint8_t tmp;
    for (uint8_t i = 0; i < LED_COUNT; ++i) {
        uint8_t index = i * 3;
        tmp = led_buffer[index];
        led_buffer[index] = led_buffer[index + 1];
        led_buffer[index + 1] = tmp;
    }
    output_grb_pc2(led_buffer, sizeof(led_buffer));
}

void save_modified() {
    for (uint8_t i = 0; i < DEVICE_COUNT; ++i) {
        if (device_modified[i]) {
            save_settings(&settings[i], i);
            device_modified[i] = 0;
        }
    }
}

void init_avr() {
    /* Setup ports */
    DDRC = 0x04;
    DDRB = 0x20;
    DDRD = 0x1e;

    /* Initialize timer 1 to fire an interrupt 64 times per second */
    OCR1A = 31250;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11);
    TIMSK1 |= (1 << OCIE1A);

    /* Global interrupt enable */
    sei();
    wdt_enable(WDTO_60MS);

    /* Setup UART */
    init_uart();

    /* Set up globals */
    uart_data = NULL;
    new_frame = 1;
    frame = 0;
    uart_invalid_count = 0;
    reboot_frame = UINT32_MAX;
    save_frame = UINT32_MAX;
    read_all_settings(settings);
    memset(led_buffer, 0, LED_COUNT * 3);
    setup_effects(device_effects);
}

int main() {
    init_avr();

    for (;;) {
        if (uart_invalid_count > 100) {
            reboot();
        }

        if (!uart_buffer_is_empty()) {
            handle_uart();
        }

        if (new_frame && !(frame % TIMER_DIVIDE)) {
            new_frame = 0;

            if (uart_state == STATE_NONE) {
                uart_send(UART_BUSY);
                output_leds();
                uart_send(UART_FREE);
            }
            wdt_reset();

            if (frame >= reboot_frame) {
                reboot();
            }

            if (frame >= save_frame) {
                save_modified();
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
    frame++;
}
