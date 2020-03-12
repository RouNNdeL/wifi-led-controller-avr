//
// Created by Krzysiek on 2019-07-30.
//

#ifndef AVR_MEMORY_H
#define AVR_MEMORY_H

#include "stdint.h"
#include "config.h"

#define FLAG_ON (1 << 0)

typedef struct
{
    uint8_t brightness;
    uint8_t flags;
    uint8_t color[3];
} __attribute__((packed)) device_settings;

typedef struct
{
    uint8_t effect;
    uint8_t color_count;
    uint8_t timing[TIME_COUNT];
    uint8_t args[ARG_COUNT];
    uint8_t colors[COLOR_COUNT * 3];
} __attribute__((packed)) device_effect;

#endif //AVR_MEMORY_H
