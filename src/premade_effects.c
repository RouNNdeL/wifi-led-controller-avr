//
// Created by krzys on 21/01/13.
//

#include <memory.h>
#include "premade_effects.h"

void rotating_rainbow(device_effect* effect) {
    effect->effect = RAINBOW;
    effect->args[ARG_BIT_PACK] = SMOOTH;
    effect->args[ARG_RAINBOW_BRIGHTNESS] = UINT8_MAX;
    effect->args[ARG_RAINBOW_SOURCES] = 1;
    effect->timing[TIME_ROTATION] = FPS * 2;
    effect->flags = (1 << TIME_ROTATION);
    effect->color_count = 0;
}

void police_spinning(device_effect* effect, uint8_t piece_count) {
    effect->effect = PIECES;
    effect->args[ARG_BIT_PACK] = SMOOTH;
    effect->args[ARG_COLOR_CYCLES] = 1;
    effect->args[ARG_PIECES_PIECE_COUNT] = piece_count;
    effect->args[ARG_PIECES_COLOR_COUNT] = 2;
    effect->timing[TIME_ROTATION] = FPS;
    effect->timing[TIME_ON] = 1;
    effect->flags = (1 << TIME_ROTATION);
    set_color_manuali_grb(effect->colors, 0, COLOR_RED);
    set_color_manuali_grb(effect->colors, 1, COLOR_BLUE);
    effect->color_count = 2;
}

void police_switching(device_effect* effect, uint8_t piece_count) {
    effect->effect = PIECES;
    effect->args[ARG_BIT_PACK] = SMOOTH | DIRECTION;
    effect->args[ARG_COLOR_CYCLES] = 1;
    effect->args[ARG_PIECES_PIECE_COUNT] = piece_count;
    effect->args[ARG_PIECES_COLOR_COUNT] = 2;
    effect->timing[TIME_ROTATION] = 0;
    effect->timing[TIME_ON] = FPS / 4;
    effect->flags = (1 << TIME_ON);
    set_color_manuali_grb(effect->colors, 0, COLOR_RED);
    set_color_manuali_grb(effect->colors, 1, COLOR_BLUE);
    set_color_manuali_grb(effect->colors, 2, COLOR_BLUE);
    set_color_manuali_grb(effect->colors, 3, COLOR_RED);
    effect->color_count = 4;
}

void police_blinking(device_effect* effect) {
    effect->effect = BREATHE;
    effect->args[ARG_BREATHE_START] = 0;
    effect->args[ARG_BREATHE_END] = UINT8_MAX;
    effect->args[ARG_COLOR_CYCLES] = 1;
    effect->timing[TIME_ON] = FPS / 2;
    effect->flags = (1 << TIME_ON);
    set_color_manuali_grb(effect->colors, 0, COLOR_RED);
    set_color_manuali_grb(effect->colors, 1, COLOR_BLUE);
    effect->color_count = 2;
}

void breathing(device_effect* effect) {
    breathing_full(effect, 0, UINT8_MAX);
}

void breathing_full(device_effect* effect, uint8_t min, uint8_t max) {
    effect->effect = BREATHE;
    effect->args[ARG_BREATHE_START] = min;
    effect->args[ARG_BREATHE_END] = max;
    effect->args[ARG_COLOR_CYCLES] = 1;
    effect->timing[TIME_FADEIN] = FPS;
    effect->timing[TIME_FADEOUT] = FPS;
    effect->flags = EFFECT_FLAG_TIME_FADEIN | EFFECT_FLAG_TIME_FADEOUT | EFFECT_FLAG_INHERIT_COLOR;
    set_color_manuali_grb(effect->colors, 0, COLOR_BLACK);
    effect->color_count = 1;
}