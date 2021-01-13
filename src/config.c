#include <memory.h>
#include <premade_effects.h>

void setup_effects(device_effect effects[EFFECT_COUNT]) {
    rotating_rainbow(&effects[0]);
    police_spinning(&effects[1], 4);
    police_blinking(&effects[2]);
    breathing_full(&effects[3], 128, UINT8_MAX);
}