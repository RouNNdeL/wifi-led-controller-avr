//
// Created by krzys on 21/01/13.
//

#ifndef AVR_PREMADE_EFFECTS_H
#define AVR_PREMADE_EFFECTS_H

void rotating_rainbow(device_effect *effect);

void police_spinning(device_effect *effect, uint8_t piece_count);

void police_switching(device_effect *effect, uint8_t piece_count);

void police_blinking(device_effect* effect);

void breathing(device_effect* effect);

void breathing_full(device_effect* effect, uint8_t min, uint8_t max);


#endif //AVR_PREMADE_EFFECTS_H
