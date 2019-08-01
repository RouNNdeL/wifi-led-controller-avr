//
// Created by Krzysiek on 2019-07-30.
//

#ifndef AVR_MEMORY_H
#define AVR_MEMORY_H

#include "../../../../../.platformio/packages/toolchain-atmelavr/avr/include/stdint.h"
#include "config.h"

typedef struct
{
    uint8_t brightness[DEVICE_COUNT];
    uint8_t flags[DEVICE_COUNT];
    uint8_t color[3 * DEVICE_COUNT];
    uint8_t current_device_profile[DEVICE_COUNT];
    uint8_t profile_count;
    uint8_t current_profile;
    uint8_t auto_increment;
    int8_t profiles[PROFILE_COUNT][DEVICE_COUNT];
    uint8_t profile_flags[PROFILE_COUNT];
} __attribute__((packed)) global_settings;

typedef struct
{
    uint8_t effect;
    uint8_t color_count;
    uint8_t timing[TIME_COUNT];
    uint8_t args[ARG_COUNT];
    uint8_t colors[COLOR_COUNT * 3];
} __attribute__((packed)) device_effect;

#endif //AVR_MEMORY_H
