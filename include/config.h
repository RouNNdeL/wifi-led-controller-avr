//
// Created by Krzysiek on 2019-07-30.
//

#ifndef AVR_CONFIG_H
#define AVR_CONFIG_H

#include <stdint.h>

// Devices
#define VIRTUAL_DEVICES {1, 54,54}
#define LED_COUNT 109
#define DEVICE_COUNT 3

// Effects
#define EFFECT_COUNT 4
#define COLOR_COUNT 16
#define ARG_COUNT 6
#define TIME_COUNT 6

#define UART_BUFFER_SIZE 64

#if UART_BUFFER_SIZE <= 255
typedef uint8_t uart_buffer_t;
#else
typedef uint16_t uart_buffer_t;
#endif

#if UART_BUFFER_SIZE <= 255
typedef uint8_t uart_buffer_t;
#else
typedef uint16_t uart_buffer_t;
#endif

/* FPS * TIMER_DIVIDE = 64 */
#define FPS 64
#define TIMER_DIVIDE 1
/* Setting this to a low value may result in the EEPROM's write capabilities to wear out quickly */
#define SAVE_DELAY (5 * FPS)
#define REBOOT_DELAY (FPS / 10)
#define TRANSITION_FRAMES 32
#define TRANSITION_QUICK_FRAMES 20

/*
 * The transition doesn't look well, probably due to more calculations taking place,
 * hence the option to disable it
 */
#define TRANSITION_EFFECTS 0

#endif //AVR_CONFIG_H
