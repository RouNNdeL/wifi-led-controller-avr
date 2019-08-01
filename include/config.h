//
// Created by Krzysiek on 2019-07-30.
//

#ifndef AVR_CONFIG_H
#define AVR_CONFIG_H

#define VERSION_CODE 1
#define VERSION_NAME "0.0.1"

#define TRANSITION_FRAMES 32
#define TRANSITION_QUICK_FRAMES 20
/*
 * The tradition doesn't look well, probably due to more calculations taking place,
 * hence the option to disable it
 */
#define TRANSITION_EFFECTS 0

#if TRANSITION_FRAMES <= 255 && TRANSITION_QUICK_FRAMES <= 255
typedef uint8_t transition_t;
#else
typedef uint16_t transition_t;
#endif

#ifndef VIRTUAL_DEVICES
#define VIRTUAL_DEVICES {60}
#endif /* VIRTUAL_DEVICES */

#ifndef LED_PIN
#define LED_PIN 4
#endif /* LED_PIN */

#ifndef LED_COUNT
#define LED_COUNT 60
#endif /* LED_COUNT */

#ifndef PROFILE_COUNT
#define PROFILE_COUNT 8
#endif /* PROFILE_COUNT */

#ifndef DEVICE_PROFILE_COUNT
#define DEVICE_PROFILE_COUNT 8
#endif /* DEVICE_PROFILE_COUNT */

#ifndef DEVICE_COUNT
#define DEVICE_COUNT 1
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
