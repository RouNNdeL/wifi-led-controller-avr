//
// Created by Krzysiek on 2019-07-30.
//

#ifndef AVR_MEMORY_H
#define AVR_MEMORY_H

#include <effects.h>
#include "stdint.h"
#include "config.h"

#define FLAG_ON (1 << 0)
#define FLAG_EFFECT_ON (1 << 1)
#define FLAG_LOG_BRIGHTNESS (1 << 2)

typedef struct {
    uint8_t brightness;
    uint8_t flags;
    uint8_t color[3];
    uint8_t effect;
    uint8_t effect_speed;
} __attribute__((packed)) device_settings;

typedef struct {
    effect effect;
    uint8_t color_count;
    uint16_t timing[TIME_COUNT];
    uint8_t timing_mask; // Used to determine which timing can be scaled by effect speed
    uint8_t args[ARG_COUNT];
    uint8_t colors[COLOR_COUNT * 3];
} __attribute__((packed)) device_effect;

void read_settings(device_settings* settings, uint8_t device_index);

void save_settings(device_settings* settings, uint8_t device_index);

void read_all_settings(device_settings settings[DEVICE_COUNT]);

void setup_effects(device_effect effects[EFFECT_COUNT]);

#endif //AVR_MEMORY_H
