#include <avr/io.h>
#include <string.h>
#include "effects.h"

/* Credit: https://github.com/FastLED/FastLED */
uint8_t scale8(uint8_t i, uint8_t scale) {
    asm volatile(
#define FASTLED_SCALE8_FIXED 0
#if (FASTLED_SCALE8_FIXED == 1)
    // Multiply 8-bit i * 8-bit scale, giving 16-bit r1,r0
        "mul %0, %1          \n\t"
        // Add i to r0, possibly setting the carry flag
        "add r0, %0         \n\t"
        // load the immediate 0 into i (note, this does _not_ touch any flags)
        "ldi %0, 0x00       \n\t"
        // walk and chew gum at the same time
        "adc %0, r1          \n\t"
#else
    /* Multiply 8-bit i * 8-bit scale, giving 16-bit r1,r0 */
    "mul %0, %1          \n\t"
    /* Move the high 8-bits of the product (r1) back to i */
    "mov %0, r1          \n\t"
    /* Restore r1 to "0"; it's expected to always be that */
    #endif
    "clr __zero_reg__    \n\t"

    : "+a" (i)      /* writes to i */
    : "a"  (scale)  /* uses scale */
    : "r0", "r1"    /* clobbers r0, r1 */ );

    /* Return the result */
    return i;
}

void set_color(uint8_t *p_buf, uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
    uint16_t index = 3 * led;
    p_buf[index++] = r;
    p_buf[index++] = g;
    p_buf[index] = b;
}

void set_all_colors(uint8_t *p_buf, uint8_t r, uint8_t g, uint8_t b, uint8_t count, uint8_t grb) {
    if(grb) {
        for(uint8_t i = 0; i < count; ++i) {
            set_color_grb(p_buf, i, r, g, b);
        }
    } else {
        for(uint8_t i = 0; i < count; ++i) {
            set_color(p_buf, i, r, g, b);
        }
    }
}

void cross_fade(uint8_t *color, uint8_t *colors, uint8_t n_color, uint8_t m_color, uint8_t progress) {
    if(colors[n_color] > colors[m_color])
        color[0] = colors[n_color] - scale8(colors[n_color++] - colors[m_color++], progress);
    else
        color[0] = colors[n_color] + scale8(colors[m_color++] - colors[n_color++], progress);

    if(colors[n_color] > colors[m_color])
        color[1] = colors[n_color] - scale8(colors[n_color++] - colors[m_color++], progress);
    else
        color[1] = colors[n_color] + scale8(colors[m_color++] - colors[n_color++], progress);

    if(colors[n_color] > colors[m_color])
        color[2] = colors[n_color] - scale8(colors[n_color] - colors[m_color], progress);
    else
        color[2] = colors[n_color] + scale8(colors[m_color] - colors[n_color], progress);
}

void cross_fade_values(uint8_t *color, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2,
                       uint8_t progress) {
    if(r1 > r2)
        color[0] = r1 - (r1 - r2) * (uint16_t) progress / UINT8_MAX;
    else
        color[0] = r1 + (r2 - r1) * (uint16_t) progress / UINT8_MAX;

    if(g1 > g2)
        color[1] = g1 - (g1 - g2) * (uint16_t) progress / UINT8_MAX;
    else
        color[1] = g1 + (g2 - g1) * (uint16_t) progress / UINT8_MAX;

    if(b1 > b2)
        color[2] = b1 - (b1 - b2) * (uint16_t) progress / UINT8_MAX;
    else
        color[2] = b1 + (b2 - b1) * (uint16_t) progress / UINT8_MAX;
}

void cross_fade_bright(uint8_t *color, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2,
                       uint8_t progress) {
    uint8_t r_max = r1 > r2 ? r1 : r2;
    uint8_t g_max = g1 > g2 ? g1 : g2;
    uint8_t b_max = b1 > b2 ? b1 : b2;

    if(progress <= INT8_MAX) {
        cross_fade_values(color, r1, g1, b1, r_max, g_max, b_max, progress * 2);
    } else {
        cross_fade_values(color, r_max, g_max, b_max, r2, g2, b2, (progress - INT8_MAX) * 2);
    }
}

void rainbow_at_progress_full(uint8_t *color, uint16_t progress, uint8_t brightness, uint8_t grb) {
    if(progress <= 21845) {
        if(grb)
            cross_fade_bright(color, COLOR_GREEN, COLOR_BLUE, progress / 86);
        else
            cross_fade_bright(color, COLOR_RED, COLOR_BLUE, progress / 86);
    } else if((progress -= 21845) <= 21845) {
        if(grb)
            cross_fade_bright(color, COLOR_BLUE, COLOR_RED, progress / 86);
        else
            cross_fade_bright(color, COLOR_BLUE, COLOR_GREEN, progress / 86);
    } else if((progress -= 21845) <= 21845) {
        if(grb)
            cross_fade_bright(color, COLOR_RED, COLOR_GREEN, progress / 86);
        else
            cross_fade_bright(color, COLOR_GREEN, COLOR_RED, progress / 86);
    }

    set_color_manual(color, color_brightness(brightness, color_from_buf(color)));
}

void rainbow_at_progress(uint8_t *color, uint16_t progress, uint8_t brightness, uint8_t grb) {
    if(progress <= 21845) {
        if(grb)
            cross_fade_values(color, COLOR_GREEN, COLOR_BLUE, progress / 86);
        else
            cross_fade_values(color, COLOR_RED, COLOR_BLUE, progress / 86);
    } else if((progress -= 21845) <= 21845) {
        if(grb)
            cross_fade_values(color, COLOR_BLUE, COLOR_RED, progress / 86);
        else
            cross_fade_values(color, COLOR_BLUE, COLOR_GREEN, progress / 86);
    } else if((progress -= 21845) <= 21845) {
        if(grb)
            cross_fade_values(color, COLOR_RED, COLOR_GREEN, progress / 86);
        else
            cross_fade_values(color, COLOR_GREEN, COLOR_RED, progress / 86);
    }

    set_color_manual(color, color_brightness(brightness, color_from_buf(color)));
}

void rotate_buf(uint8_t *leds, uint8_t led_count, uint16_t rotation_progress, uint8_t start_led, uint16_t piece_leds,
                uint8_t bit_pack, uint8_t *colors, uint8_t color_count) {
    /* Which LED is the start led */
    uint8_t led_offset = (rotation_progress / (UINT16_MAX / led_count)) % led_count;
    /* What's left from the previous LED */
    uint8_t led_carry = (uint32_t) rotation_progress * UINT8_MAX / (UINT16_MAX / led_count) % UINT8_MAX;
    /* How many LEDs per piece (*255) */
    uint16_t current_leds = 0;
    uint8_t color = 0;

    led_offset = (bit_pack & DIRECTION) ? led_offset : led_count - led_offset - 1;
    led_carry = (bit_pack & DIRECTION) ? led_carry : UINT8_MAX - led_carry;

    for(uint8_t j = 0; j < led_count; ++j) {
        //TODO: Add direction argument support
        uint8_t index = (led_count + j + led_offset - start_led) % led_count * 3;

        /* If we're at the first LED of a certain color and led_carry != 0 crossfade with the previous color */
        if(current_leds == 0 && led_carry && (bit_pack & SMOOTH)) {
            cross_fade(leds + index, colors, color * 3, ((color + color_count - 1) % color_count) * 3, led_carry);

            current_leds = led_carry;
        } else if((current_leds += UINT8_MAX) <= piece_leds) {
            set_color_manual(leds + index, color_from_buf(colors + color * 3));
        } else {
            color = (color + 1) % color_count; /* Next color */
            current_leds = 0; /* Reset current counter */
            j--; /* Backtrack to crossfade that LED */
        }
    }
}


/**
 * @param effect - effect to calculate
 * @param color - an array of length 3 that the color is going to be assigned to
 * @param frame - a frame to calculate the color for
 * @param times - timing arguments resolved to frames
 * @param colors - an array of length 24 containing 16*4 colors
 *               in a RGB order
 * @param color_count - how many colors are in use
 * @return - a color to be displayed at a given frame
 */
void simple_effect(effect effect, uint8_t *color, uint32_t frame, uint16_t *times, uint8_t *args, uint8_t *colors,
                   uint8_t color_count, uint8_t grb) {
    uint32_t sum = times[0] + times[1] + times[2] + times[3];
    uint32_t d_time = frame % sum;
    uint8_t n_color = ((frame / sum / args[ARG_COLOR_CYCLES]) % color_count);
    uint8_t m_color = (n_color == color_count - 1) ? 0 : n_color + 1;
    n_color *= 3;
    m_color *= 3;

    if((d_time) < times[TIME_OFF]) {
        //<editor-fold desc="Off">
        color[0] = colors[n_color++] * args[ARG_BREATHE_START] / UINT8_MAX;
        color[1] = colors[n_color++] * args[ARG_BREATHE_START] / UINT8_MAX;
        color[2] = colors[n_color] * args[ARG_BREATHE_START] / UINT8_MAX;
        //</editor-fold>
    } else if((d_time -= times[TIME_OFF]) < times[TIME_FADEIN]) {
        //<editor-fold desc="Fade in">
        uint8_t delta_brightness = args[ARG_BREATHE_END] - args[ARG_BREATHE_START];
        uint8_t progress = d_time * delta_brightness / times[TIME_FADEIN] + args[ARG_BREATHE_START];

        set_color_manual(color, color_brightness(progress, color_from_buf(colors + n_color)));
        //</editor-fold>
    } else if((d_time -= times[TIME_FADEIN]) < times[TIME_ON]) {
        //<editor-fold desc="On">
        if(effect == BREATHE) {
            set_color_manual(color, color_brightness(args[ARG_BREATHE_END], color_from_buf(colors + n_color)));
        } else {
            set_color_manual(color, color_from_buf(colors + n_color));
        }
        //</editor-fold>
    } else if((d_time -= times[TIME_ON]) < times[TIME_FADEOUT]) {
        //<editor-fold desc="Fade out">
        if(effect == BREATHE) {
            uint8_t delta_brightness = args[ARG_BREATHE_END] - args[ARG_BREATHE_START];
            uint8_t progress = args[ARG_BREATHE_END] - d_time * delta_brightness / times[TIME_FADEOUT];

            set_color_manual(color, color_brightness(progress, color_from_buf(colors + n_color)));
        } else {
            if(effect == FADE) {
                uint8_t progress = d_time * UINT8_MAX / times[TIME_FADEOUT];
                cross_fade(color, colors, n_color, m_color, progress);
            } else {
                uint16_t progress = d_time * UINT16_MAX / times[TIME_FADEOUT];
                rainbow_at_progress_full(color, progress, args[ARG_RAINBOW_BRIGHTNESS], grb);
            }
        }
        //</editor-fold>
    }
}


/**
 * Function to calculate the effects for addressable LEDs
 *
 * Arguments for effects:<ul>
 * <li>FILL, FADE - {bit_packed*, NONE, NONE, NONE}</li>
 * <li>RAINBOW - {bit_packed*, brightness, sources, NONE}</li>
 * <li>PIECES - {bit_packed*, color_count, piece_count, NONE}</li>
 * <li>ROTATING - {bit_packed*, color_count, element_count, led_count}</li></ul>
 * 
 * * - We pack 1 bit values to allow for more arguments:<ul>
 * <li>DIRECTION - 0</li>
 * <li>SMOOTH - 1</li>
 * <li>MODE - 2 (for RAINBOW only)</li></ul>
 *
 * @param effect - effect to calculate
 * @param leds - a pointer to an RGB array
 * @param led_count - number of LEDs in the 'leds' pointer
 * @param offset - offset for the pointer
 * @param frame - a frame to calculate the effect for
 * @param times - timing arguments resolved to frames
 * @param args - additional arguments for the effects, described above
 * @param colors - an array of length 24 containing 16*4 colors
 *               in a RGB order
 * @param color_count - how many colors are in use
 */
void digital_effect(effect effect, uint8_t *leds, uint8_t led_count, uint8_t start_led, uint32_t frame, uint16_t *times,
                    uint8_t *args, uint8_t *colors, uint8_t color_count) {
    if(effect == BREATHE || effect == FADE || (effect == RAINBOW && (args[ARG_BIT_PACK] & RAINBOW_SIMPLE))) {
        uint8_t color[3];
        simple_effect(effect, color, frame, times, args, colors, color_count, 1);
        set_all_colors(leds, color[0], color[1], color[2], led_count, 0);
        return;
    }
    if(effect == FILLING_FADE || effect == FILL || effect == RAINBOW) {
        //<editor-fold desc="FILL, FILLING_FADE, RAINBOW">
        uint32_t sum = times[0] + times[1] + times[2] + times[3];
        uint32_t d_time = frame % sum;
        uint8_t n_color = ((frame / sum / args[ARG_COLOR_CYCLES]) % (color_count / args[ARG_FILL_COLOR_COUNT])) *
                          args[ARG_FILL_COLOR_COUNT];
        uint8_t m_color = (n_color + args[ARG_FILL_COLOR_COUNT]) % color_count;
        n_color *= 3;
        m_color *= 3;

        if((d_time) < times[TIME_OFF]) {
            set_all_colors(leds, 0x00, 0x00, 0x00, led_count, 0);
        } else if((d_time -= times[TIME_OFF]) < times[TIME_FADEIN]) {
            //<editor-fold desc="Fade in">
            /* A 16bit replace for a float*/
            uint16_t progress = d_time * UINT16_MAX / times[TIME_FADEIN];

            if(effect == FILL) {

                uint8_t piece_leds = led_count / args[ARG_FILL_PIECE_COUNT];

                /* Number from 0-255*led_count, used to make the effect smooth*/
                uint16_t led_progress_base = (progress * (uint32_t) piece_leds) / UINT8_MAX;
                uint16_t led_progress_current;
                int8_t piece = -1;
                uint8_t arg_number = 0;

                for(uint8_t i = 0; i < led_count; ++i) {
                    if(i % piece_leds == 0) {
                        led_progress_current = led_progress_base;
                        piece++;
                        if(piece > 8) {
                            piece = 0;
                            arg_number = 1;
                        }
                    }

                    uint8_t direction = (arg_number ? args[ARG_FILL_PIECE_DIRECTIONS2] :
                                         args[ARG_FILL_PIECE_DIRECTIONS1]) & (1 << piece);
                    uint8_t index = (((direction ? i : led_count - i - 1))
                                     % piece_leds + piece * piece_leds + start_led) % led_count * 3;

                    uint8_t n_color_for_piece = n_color + 3 * (((piece + 8 * arg_number)) % args[ARG_FILL_COLOR_COUNT]);
                    if(led_progress_current >= UINT8_MAX) {
                        set_color_manual(leds + index, color_from_buf(colors + n_color_for_piece));

                        led_progress_current -= UINT8_MAX;
                    } else if(led_progress_current > 0 && (args[ARG_BIT_PACK] & SMOOTH)) {
                        set_color_manual(leds + index, color_brightness(led_progress_current,
                                                                        color_from_buf(colors + n_color_for_piece)));

                        led_progress_current = 0;
                    } else {
                        set_color_manual(leds + index, COLOR_BLACK);
                    }
                }
            }
            //</editor-fold>
        } else if((d_time -= times[TIME_FADEIN]) < times[TIME_ON]) {
            //<editor-fold desc="On">
            uint8_t piece_leds = led_count / args[ARG_FILL_PIECE_COUNT];
            int8_t piece = -1;

            for(uint8_t i = 0; i < led_count; ++i) {
                if(i % piece_leds == 0) {
                    piece++;
                }

                uint8_t index = (i + start_led) % led_count * 3;

                uint8_t n_color_for_piece = n_color + 3 * (piece % args[ARG_FILL_COLOR_COUNT]);
                set_color_manual(leds + index, color_from_buf(colors + n_color_for_piece));
            }
            //</editor-fold>
        } else if((d_time -= times[TIME_ON]) < times[TIME_FADEOUT]) {
            //<editor-fold desc="Fade out">
            uint16_t progress = UINT16_MAX - d_time * UINT16_MAX / times[TIME_FADEOUT];

            uint8_t piece_leds = led_count / args[ARG_FILL_PIECE_COUNT];
            uint16_t led_progress_base = (progress * (uint32_t) piece_leds) / UINT8_MAX;

            uint16_t led_progress_current = led_progress_base;
            int8_t piece = -1;
            uint8_t arg_number = 0;

            for(uint8_t i = 0; i < led_count; ++i) {
                if(i % piece_leds == 0) {
                    led_progress_current = led_progress_base;
                    piece++;
                    if(piece > 8) {
                        piece = 0;
                        arg_number = 1;
                    }
                }

                uint8_t direction = ((arg_number ? args[ARG_FILL_PIECE_DIRECTIONS2] :
                                      args[ARG_FILL_PIECE_DIRECTIONS1]) & (1 << piece)) ? 1 : 0;

                if(effect == FILLING_FADE && (args[ARG_BIT_PACK] & FILL_FADE_RETURN) && (frame / sum) % 2 == 0)
                    direction = direction ? 0 : 1;
                uint8_t index =
                        (((direction ^ (args[ARG_BIT_PACK] & FILL_FADE_RETURN ? 1 : 0) ? led_count - i - 1 : i))
                         % piece_leds + piece * piece_leds + start_led) % led_count * 3;

                uint8_t n_color_for_piece = n_color + 3 * (((piece + 8 * arg_number)) % args[ARG_FILL_COLOR_COUNT]);
                uint8_t m_color_for_piece = m_color + 3 * (((piece + 8 * arg_number)) % args[ARG_FILL_COLOR_COUNT]);

                if(led_progress_current >= UINT8_MAX) {
                    set_color_manual(leds + index, color_from_buf(colors + n_color_for_piece));

                    led_progress_current -= UINT8_MAX;
                } else {
                    if(led_progress_current > 0 && (args[ARG_BIT_PACK] & SMOOTH)) {
                        if(effect == FILL) {
                            set_color_manual(leds + index, color_brightness(led_progress_current, color_from_buf(
                                    colors + n_color_for_piece)));
                        } else {
                            if(args[ARG_BIT_PACK] & FADE_SMOOTH) {
                                uint8_t faded[6];
                                set_color_manual(faded, color_from_buf(colors + n_color_for_piece));
                                cross_fade(faded + 3, colors, m_color_for_piece, n_color_for_piece,
                                           progress / UINT8_MAX);
                                cross_fade(leds + index, faded, 3, 0, led_progress_current);
                            } else {
                                cross_fade(leds + index, colors, m_color_for_piece, n_color_for_piece,
                                           led_progress_current);
                            }
                        }

                        led_progress_current = 0;
                    } else {
                        if(effect == FILL) {
                            set_color_manual(leds + index, COLOR_BLACK);
                        } else {
                            if(args[ARG_BIT_PACK] & FADE_SMOOTH) {
                                cross_fade(leds + index, colors, m_color_for_piece, n_color_for_piece,
                                           progress / UINT8_MAX);
                            } else {
                                set_color_manual(leds + index, color_from_buf(colors + m_color_for_piece));
                            }
                        }
                    }
                }
            }
            //</editor-fold>
        }
        if(times[TIME_ROTATION] && effect != RAINBOW) {
            uint16_t rotation_progress =
                    ((uint32_t) (frame % times[TIME_ROTATION]) * UINT16_MAX) / times[TIME_ROTATION];
            uint8_t backup[led_count * 3];
            memcpy(backup, leds, led_count * 3);
            rotate_buf(leds, led_count, rotation_progress, 0, UINT8_MAX, args[ARG_BIT_PACK], backup, led_count);
        }
        if(effect == RAINBOW) {
            //TODO: Possibly replace with SPECTRUM (if it is faster)
            uint16_t rotation_progress = times[TIME_ROTATION] ?
                                         ((uint32_t) (frame % times[TIME_ROTATION]) * UINT16_MAX) / times[TIME_ROTATION]
                                                              : 0;
            uint16_t led_progress_base = UINT16_MAX / led_count;
            for(uint8_t i = 0; i < led_count; ++i) {
                uint8_t index =
                        (((args[ARG_BIT_PACK] & DIRECTION) ? i : led_count - i - 1) + start_led) % led_count * 3;
                uint16_t d_progress =
                        (rotation_progress + i * led_progress_base) * args[ARG_RAINBOW_SOURCES] % UINT16_MAX;
                if(args[ARG_BIT_PACK] & RAINBOW_MODE)
                    rainbow_at_progress_full(leds + index, d_progress, args[ARG_RAINBOW_BRIGHTNESS], 1);
                else
                    rainbow_at_progress(leds + index, d_progress, args[ARG_RAINBOW_BRIGHTNESS], 1);
            }
        }
        //</editor-fold>
    } else if(effect == PARTICLES) {
        //<editor-fold desc="PARTICLES">
        uint16_t led_bytes = led_count * UINT8_MAX;
        uint16_t particle_offset =
                (uint32_t) (led_bytes + args[ARG_PARTICLES_SIZE] * UINT8_MAX) * times[TIME_PARTICLE_DELAY] /
                times[TIME_PARTICLE_SPEED];
        particle_offset = particle_offset ? particle_offset
                                          : 1; /* Just in case the user entries TIME_PARTICLE_DELAY = 0 */
        uint16_t speed_in_offset = ((uint32_t) times[TIME_PARTICLE_SPEED] * particle_offset) / led_bytes;
        uint16_t d_time = frame % speed_in_offset;
        int16_t particle_progress = (led_bytes + args[ARG_PARTICLES_SIZE] * UINT8_MAX - particle_offset) +
                                    (uint32_t) d_time * particle_offset / speed_in_offset;

        /* Precalculated array that determines the brighntess at each point of the particle */
        uint8_t particle[args[ARG_PARTICLES_SIZE] + 2];
        particle[0] = 0;
        for(uint8_t i = 1; i <= args[ARG_PARTICLES_SIZE]; ++i) {
            particle[i] = i * UINT8_MAX / args[ARG_PARTICLES_SIZE];
            particle[i] = actual_brightness(particle[i]);
        }
        particle[args[ARG_PARTICLES_SIZE] + 1] = 0;

        uint16_t led_cpy[led_count * 3];
        set_all_colors(led_cpy, 0, 0, 0, led_count * 2, 0);
        uint8_t particle_count = 0;

        uint8_t n_color = frame / speed_in_offset % color_count * 3;
        for(;;) {
            //<editor-fold desc="Single particle">
            for(uint8_t i = 0; i < led_count; ++i) {
                uint8_t index =
                        (((args[ARG_BIT_PACK] & DIRECTION) ? i : led_count - i - 1) + start_led) % led_count * 3;

                /* We are in front of the the particle */
                if((i + args[ARG_PARTICLES_SIZE] + 1) * UINT8_MAX <= particle_progress) {
                    /* Do nothing */
                    //set_color_manual(led + index, COLOR_BLACK);
                } else if(i * UINT8_MAX <= particle_progress) {
                    uint8_t p_index = i - (particle_progress / UINT8_MAX - args[ARG_PARTICLES_SIZE]);
                    uint8_t _colors[6];

                    set_color_manual(_colors, color_brightness(particle[p_index], color_from_buf(colors + n_color)));
                    set_color_manual(_colors + 3,
                                     color_brightness(particle[p_index + 1], color_from_buf(colors + n_color)));

                    uint8_t color[3];
                    cross_fade(color, _colors, 3, 0,
                               args[ARG_BIT_PACK] & SMOOTH ? particle_progress % UINT8_MAX : UINT8_MAX);
                    led_cpy[index++] += color[0];
                    led_cpy[index++] += color[1];
                    led_cpy[index] += color[2];
                } else /* We are behind the particle */
                {
                    /* Do nothing */
                    //set_color_manual(led + index, COLOR_BLACK);
                }
            }
            //</editor-fold>
            if(particle_progress < particle_offset || times[TIME_PARTICLE_DELAY] > times[TIME_PARTICLE_SPEED]) break;
            particle_progress -= particle_offset;
            particle_count++;
            n_color = (n_color + 3) % (color_count * 3);
        }

        for(uint8_t i = 0; i < led_count * 3; i++) {
            leds[i] = (uint8_t) led_cpy[i];
        }
        //</editor-fold>
    } else if(effect == SPECTRUM) {
        //<editor-fold desc="SPECTRUM">
        /* How much progress per one LED */
        uint8_t progress_per_led = UINT8_MAX / led_count * args[ARG_SPECTRUM_COLOR_COUNT];
        uint16_t sum = times[TIME_ON] + times[TIME_FADEOUT];
        uint16_t d_time = frame % sum;

        uint8_t current_progress = times[TIME_ROTATION] ? (frame % times[TIME_ROTATION]) * UINT8_MAX *
                                                          args[ARG_SPECTRUM_COLOR_COUNT] / times[TIME_ROTATION] %
                                                          UINT8_MAX : 0;

        /* We need to know the current color batch to properly use ARG_SPECTRUM_MODES bit array */
        uint8_t run_count = color_count / args[ARG_SPECTRUM_COLOR_COUNT];
        uint8_t run = (frame / sum / args[ARG_COLOR_CYCLES]) % run_count;
        uint8_t base_color = run * args[ARG_SPECTRUM_COLOR_COUNT];
        uint8_t n_color = times[TIME_ROTATION] ?
                          (frame % times[TIME_ROTATION]) * args[ARG_SPECTRUM_COLOR_COUNT] / times[TIME_ROTATION] : 0;
        uint8_t m_color = (n_color + 1) % args[ARG_SPECTRUM_COLOR_COUNT];

        for(uint8_t i = 0; i < led_count; ++i) {
            uint8_t index = (((args[ARG_BIT_PACK] & DIRECTION) ? i : led_count - i - 1) + start_led) % led_count * 3;

            if(args[ARG_SPECTRUM_MODES] & (1 << run))
                cross_fade(leds + index, colors, (base_color + n_color) * 3, (base_color + m_color) * 3,
                           current_progress);
            else
                cross_fade_bright(leds + index, color_from_buf(colors + (base_color + n_color) * 3),
                                  color_from_buf(colors + (base_color + m_color) * 3), current_progress);


            if(current_progress >= UINT8_MAX - progress_per_led) {
                /* Increment the color */
                n_color = (n_color + 1) % args[ARG_SPECTRUM_COLOR_COUNT];
                m_color = (m_color + 1) % args[ARG_SPECTRUM_COLOR_COUNT];
                current_progress = progress_per_led - (UINT8_MAX - current_progress);
            } else {
                current_progress += progress_per_led;
            }
        }


        /*
         * Due to a possibility of blending modes being different between adjacent sets of colors
         * we cannot simply precalculate the faded values and apply spectrum in between.
         * Instead we compute 2 sets independently and then fade between them
         * */
        if(d_time > times[TIME_ON] && run_count > 1) {
            uint8_t fade_progress = (d_time - times[TIME_ON]) * UINT8_MAX / times[TIME_FADEOUT];
            uint8_t tmp_color[3];

            run = (run + 1) % run_count;
            base_color = (base_color + args[ARG_SPECTRUM_COLOR_COUNT]) % color_count;

            for(uint8_t i = 0; i < led_count; ++i) {
                uint8_t index =
                        (((args[ARG_BIT_PACK] & DIRECTION) ? i : led_count - i - 1) + start_led) % led_count * 3;

                if(args[ARG_SPECTRUM_MODES] & (1 << run))
                    cross_fade(tmp_color, colors, (base_color + n_color) * 3, (base_color + m_color) * 3,
                               current_progress);
                else
                    cross_fade_bright(tmp_color, color_from_buf(colors + (base_color + n_color) * 3),
                                      color_from_buf(colors + (base_color + m_color) * 3), current_progress);

                cross_fade_values(leds + index, color_from_buf(leds + index), color_from_buf(tmp_color), fade_progress);


                if(current_progress >= UINT8_MAX - progress_per_led) {
                    /* Increment the color */
                    n_color = (n_color + 1) % args[ARG_SPECTRUM_COLOR_COUNT];
                    m_color = (m_color + 1) % args[ARG_SPECTRUM_COLOR_COUNT];
                    current_progress = progress_per_led - (UINT8_MAX - current_progress);
                } else {
                    current_progress += progress_per_led;
                }
            }
        }
        //</editor-fold>
    } else {
        //TODO: Change ROTATING effect to use code from PARTICLES, it'll improve handling in cases of uneven LED counts
        //<editor-fold desc="PIECES, ROTATING">
        uint16_t sum = times[TIME_ON] + times[TIME_FADEOUT];
        uint32_t d_time = frame % sum;
        uint8_t c_count = effect == ROTATING ? led_count : args[ARG_PIECES_COLOR_COUNT];
        uint8_t c_colors[c_count * 3];
        uint8_t n_color = ((frame / sum / args[ARG_COLOR_CYCLES]) % (color_count / args[ARG_PIECES_COLOR_COUNT])) *
                          args[ARG_PIECES_COLOR_COUNT];
        uint8_t m_color = (n_color + args[ARG_PIECES_COLOR_COUNT]) % color_count;
        n_color *= 3;
        m_color *= 3;

        /* Generate our colors used in this frame*/
        if((d_time) < times[TIME_ON]) {
            uint8_t count = 0;
            uint8_t c_leds = 0;
            for(uint8_t i = 0; i < c_count; ++i) {
                uint8_t index = i * 3;
                if(effect == ROTATING &&
                   (i % (led_count / args[ARG_ROTATING_ELEMENT_COUNT])) >= args[ARG_ROTATING_LED_COUNT]) {
                    set_color_manual(c_colors + index, COLOR_BLACK);
                } else {
                    uint8_t c_index = count * 3;
                    set_color_manual(c_colors + index, color_from_buf(colors + n_color + c_index));
                    c_leds++;
                    if(effect != ROTATING || c_leds >= args[ARG_ROTATING_LED_COUNT]) {
                        count = (count + 1) % args[ARG_PIECES_COLOR_COUNT];
                        c_leds = 0;
                    }
                }
            }
        } else if((d_time -= times[TIME_ON]) < times[TIME_FADEOUT]) {
            /* Crossfade based on progress if needed */
            uint8_t progress = d_time * UINT8_MAX / times[TIME_FADEOUT];
            uint8_t count = 0;
            uint8_t c_leds = 0;

            for(uint8_t i = 0; i < c_count; ++i) {
                uint8_t index = i * 3;
                if(effect == ROTATING &&
                   (i % (led_count / args[ARG_PIECES_PIECE_COUNT])) >= args[ARG_ROTATING_LED_COUNT]) {
                    set_color_manual(c_colors + index, COLOR_BLACK);
                } else {
                    uint8_t c_index = count * 3;
                    cross_fade(c_colors + index, colors, n_color + c_index, m_color + c_index, progress);
                    c_leds++;
                    if(effect != ROTATING || c_leds >= args[ARG_ROTATING_LED_COUNT]) {
                        count = (count + 1) % args[ARG_PIECES_COLOR_COUNT];
                        c_leds = 0;
                    }
                }
            }
        }

        //TODO: Possibly optimize those calculations
        uint16_t rotation_progress = times[TIME_ROTATION] ? frame % times[TIME_ROTATION] * UINT16_MAX /
                                                            times[TIME_ROTATION] : 0;
        uint16_t piece_leds = (effect == ROTATING ? 1 : led_count / args[ARG_PIECES_PIECE_COUNT]) * UINT8_MAX;

        rotate_buf(leds, led_count, rotation_progress, start_led, piece_leds, args[ARG_BIT_PACK], c_colors, c_count);
        //</editor-fold>
    }
}

uint8_t demo_music(uint8_t *fan_buf, uint8_t *pc_buf, uint8_t *gpu_buf, uint32_t frame) {
    uint8_t colors[12];
    set_color(colors, 0, 255, 0, 0);
    set_color(colors, 1, 0, 255, 0);
    set_color(colors, 2, 0, 0, 255);
    set_color(colors, 3, 255, 255, 255);

    if(frame < 32) {
        uint16_t times[] = {8, 0, 0, 24, 0};
        uint8_t args[] = {0, 0, 255, 0, 0};

        digital_effect(BREATHE, fan_buf, 12, 2, frame + 8, times, args, colors + 9, 1);
        simple_effect(BREATHE, gpu_buf, frame + 8, times, args, colors + 9, 0, 1);
        simple_effect(BREATHE, pc_buf, frame + 8, times, args, colors + 9, 0, 1);
    } else if((frame -= 32) < 28) {
        uint16_t times1[] = {6, 0, 50, 0, 0};
        uint8_t args1[] = {0, 0, 255, 0, 0, 2};

        set_color(pc_buf, 0, 0, 0, 0);
        set_color(gpu_buf, 0, 0, 0, 0);
        digital_effect(BREATHE, fan_buf, 12, 2, frame, times1, args1, colors, 1);
    } else if((frame -= 28) < 28) {
        uint16_t times[] = {6, 0, 50, 0, 0};
        uint8_t args[] = {0, 0, 255, 0, 0};

        set_color(pc_buf, 0, 0, 0, 0);
        simple_effect(BREATHE, gpu_buf, frame, times, args, colors + 3, 0, 1);
        set_all_colors(fan_buf, 0, 0, 0, 12, 1);
    } else if((frame -= 28) < 28) {
        uint16_t times[] = {6, 0, 50, 0, 0};
        uint8_t args[] = {0, 0, 255, 0, 0};

        simple_effect(BREATHE, pc_buf, frame, times, args, colors + 6, 1, 0);
        set_color(gpu_buf, 0, 0, 0, 0);
        set_all_colors(fan_buf, 0, 0, 0, 12, 1);
    } else if((frame -= 28) < 28) {
        uint16_t times[] = {6, 0, 50, 0, 0};
        uint8_t args[] = {0, 0, 255, 0, 0};

        simple_effect(BREATHE, pc_buf, frame, times, args, colors + 6, 1, 0);
        simple_effect(BREATHE, gpu_buf, frame, times, args, colors + 3, 1, 0);
        digital_effect(BREATHE, fan_buf, 12, 2, frame, times, args, colors, 1);
    } else if((frame -= 28) < 112) {
        uint16_t times1[] = {0, 16, 0, 16, 0};
        uint8_t args1[] = {0, 20, 255, 0, 0};

        simple_effect(BREATHE, pc_buf, frame + 16, times1, args1, colors + 6, 1, 0);
        simple_effect(BREATHE, gpu_buf, frame + 8, times1, args1, colors + 3, 2, 0);
        digital_effect(BREATHE, fan_buf, 12, 2, frame, times1, args1, colors, 2);
    } else if((frame -= 112) < 128) {
        uint16_t times1[] = {16, 0, 16, 0, 0};
        uint16_t times2[] = {4, 30, 0, 30, 0};
        uint8_t args1[] = {0, 0, 255, 0, 0};
        uint8_t args2[] = {SMOOTH | DIRECTION, 1, 1, 0, 0};

        set_color(pc_buf, 0, 0, 0, 0);
        simple_effect(BREATHE, gpu_buf, frame + 16, times1, args1, colors + 3, 2, 0);
        digital_effect(FILL, fan_buf, 12, 2, frame, times2, args2, colors, 2);
    } else if((frame -= 128) < 128) {
        uint16_t times1[] = {16, 0, 8, 0, 0};
        uint8_t args1[] = {0, 0, 255, 0, 0};

        simple_effect(BREATHE, pc_buf, frame + 16, times1, args1, colors + 6, 1, 0);
        simple_effect(BREATHE, gpu_buf, frame + 8, times1, args1, colors + 3, 2, 0);
        digital_effect(BREATHE, fan_buf, 12, 2, frame, times1, args1, colors, 2);
    } else if((frame -= 128) < 128) {
        uint16_t times1[] = {16, 0, 16, 0, 0};
        uint16_t times2[] = {0, 32, 0, 32, 0};
        uint8_t args1[] = {0, 0, 255, 0, 0};
        uint8_t args2[5];
        args2[ARG_BIT_PACK] = SMOOTH;
        args2[ARG_FILL_PIECE_COUNT] = 2;
        args2[ARG_FILL_COLOR_COUNT] = 1;
        args2[ARG_FILL_PIECE_DIRECTIONS1] = 0;
        args2[ARG_FILL_PIECE_DIRECTIONS2] = 0;

        set_color(pc_buf, 0, 0, 0, 0);
        simple_effect(BREATHE, gpu_buf, frame + 16, times1, args1, colors, 2, 0);
        digital_effect(FILL, fan_buf, 12, 2, frame, times2, args2, colors + 3, 2);
    } else if((frame -= 128) < 96) {
        uint16_t times1[] = {0, 16, 0, 16, 0};
        uint8_t args1[] = {0, 20, 255, 0, 0};

        simple_effect(BREATHE, pc_buf, frame + 16, times1, args1, colors + 6, 1, 0);
        simple_effect(BREATHE, gpu_buf, frame + 8, times1, args1, colors + 3, 2, 0);
        digital_effect(BREATHE, fan_buf, 12, 2, frame, times1, args1, colors, 2);
    } else if((frame -= 96) < 128) {
        uint16_t times1[] = {16, 0, 16, 0, 0};
        uint16_t times2[] = {4, 30, 0, 30, 0};
        uint8_t args1[] = {0, 0, 255, 0, 0};
        uint8_t args2[5];
        args2[ARG_BIT_PACK] = SMOOTH;
        args2[ARG_FILL_PIECE_COUNT] = 1;
        args2[ARG_FILL_COLOR_COUNT] = 1;
        args2[ARG_FILL_PIECE_DIRECTIONS1] = 0;
        args2[ARG_FILL_PIECE_DIRECTIONS2] = 0;

        simple_effect(BREATHE, pc_buf, frame + 16, times1, args1, colors + 6, 1, 0);
        simple_effect(BREATHE, gpu_buf, frame + 16, times1, args1, colors + 3, 2, 0);
        digital_effect(FILL, fan_buf, 12, 2, frame, times2, args2, colors, 2);
    } else { return 1; }
    return 0;
}

uint8_t demo_effects(uint8_t *fan_buf, uint8_t *pc_buf, uint8_t *gpu_buf, uint32_t frame) {
    if(frame < 384) {
        uint8_t colors[] = {255, 0, 0, 0, 255, 0, 0, 0, 255};

        uint16_t times[] = {256, 64, 0, 64, 0};
        uint8_t args[] = {0, 0, 255, 0, 0};

        digital_effect(BREATHE, fan_buf, 12, 2, frame + 256, times, args, colors, 1);
        simple_effect(BREATHE, gpu_buf, frame + 128, times, args, colors + 3, 1, 0);
        simple_effect(BREATHE, pc_buf, frame, times, args, colors + 6, 1, 0);
    } else if((frame -= 384) < 128) {
        uint8_t colors[] = {255, 0, 0, 0, 255, 0, 0, 0, 255};

        uint16_t times[] = {0, 64, 0, 64, 0};
        uint8_t args[] = {0, 0, 255, 0, 0};

        digital_effect(BREATHE, fan_buf, 12, 2, frame, times, args, colors, 1);
        simple_effect(BREATHE, gpu_buf, frame, times, args, colors + 3, 1, 0);
        simple_effect(BREATHE, pc_buf, frame, times, args, colors + 6, 1, 0);
    } else if((frame -= 128) < 256) {
        uint8_t colors[] = {255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0};

        uint16_t times[] = {0, 0, 64, 32, 64};
        uint8_t args[] = {SMOOTH | DIRECTION, 2, 4};

        set_color(pc_buf, 0, 0, 0, 0);
        set_color(gpu_buf, 0, 0, 0, 0);

        digital_effect(PIECES, fan_buf, 12, 2, frame, times, args, colors, 4);
    } else { return 1; }
    return 0;
}

