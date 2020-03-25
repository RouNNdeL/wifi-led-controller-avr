//
// Created by Krzysiek on 2019-07-30.
//

#ifndef AVR_CONFIG_H
#define AVR_CONFIG_H

#define VERSION_CODE 1
#define VERSION_NAME "0.0.1"

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

#define FPS 64
/* Setting this to a low value may result in the EEPROM's write capabilities to wear out quickly */
#define SAVE_DELAY 5 * FPS
#define REBOOT_DELAY FPS / 10
#define TRANSITION_FRAMES 32
#define TRANSITION_QUICK_FRAMES 20

/*
 * The transition doesn't look well, probably due to more calculations taking place,
 * hence the option to disable it
 */
#define TRANSITION_EFFECTS 0

#if TRANSITION_FRAMES <= 255 && TRANSITION_QUICK_FRAMES <= 255
typedef uint8_t transition_t;
#else
typedef uint16_t transition_t;
#endif

#ifndef VIRTUAL_DEVICES
#define VIRTUAL_DEVICES {1, 6, 6}
#endif /* VIRTUAL_DEVICES */

#ifndef LED_PIN
#define LED_PIN 4
#endif /* LED_PIN */

#ifndef LED_COUNT
#define LED_COUNT 109
#endif /* LED_COUNT */

#ifndef PROFILE_COUNT
#define PROFILE_COUNT 8
#endif /* PROFILE_COUNT */

#ifndef DEVICE_PROFILE_COUNT
#define DEVICE_PROFILE_COUNT 8
#endif /* DEVICE_PROFILE_COUNT */

#ifndef DEVICE_COUNT
#define DEVICE_COUNT 3
#endif /* DEVICE_COUNT */

#define COLOR_COUNT 16
#define ARG_COUNT 6
#define TIME_COUNT 6

#define FASTLED_SCALE8_FIXED 1

#ifndef PAGE_DEBUG
#define PAGE_DEBUG 1
#endif /* PAGE_DEBUG */

#define SERIAL_DEBUG 1

#endif //AVR_CONFIG_H
