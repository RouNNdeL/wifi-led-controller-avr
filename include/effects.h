#ifndef LEDCONTROLLER_COLOR_UTILS_H
#define LEDCONTROLLER_COLOR_UTILS_H

#include <stdint.h>

#define actual_brightness(brightness) scale8(brightness, brightness)

#define ARG_BIT_PACK 0
#define ARG_COLOR_CYCLES 5

#define DIRECTION_BIT 0
#define SMOOTH_BIT 1

#define FILL_FADE_RETURN_BIT 2
#define FADE_SMOOTH_BIT 3

#define RAINBOW_MODE_BIT 2
#define RAINBOW_SIMPLE_BIT 3

#define PARTICLES_BLEND_BIT 2

#define DIRECTION (1 << DIRECTION_BIT)
#define SMOOTH (1 << SMOOTH_BIT)

#define FILL_FADE_RETURN (1 << FILL_FADE_RETURN_BIT)
#define FADE_SMOOTH (1 << FADE_SMOOTH_BIT)

#define RAINBOW_MODE (1 << RAINBOW_MODE_BIT)
#define RAINBOW_SIMPLE (1 << RAINBOW_SIMPLE_BIT)

#define PARTICLES_BLEND (1 << PARTICLES_BLEND_BIT)

#define ARG_BREATHE_START 1
#define ARG_BREATHE_END 2

#define ARG_FILL_COLOR_COUNT 1
#define ARG_FILL_PIECE_COUNT 2
#define ARG_FILL_PIECE_DIRECTIONS1 3
#define ARG_FILL_PIECE_DIRECTIONS2 4

#define ARG_RAINBOW_BRIGHTNESS 1
#define ARG_RAINBOW_SOURCES 2

#define ARG_PIECES_COLOR_COUNT 1
#define ARG_PIECES_PIECE_COUNT 2

#define ARG_ROTATING_COLOR_COUNT 1
#define ARG_ROTATING_ELEMENT_COUNT 2
#define ARG_ROTATING_LED_COUNT 3

#define ARG_PARTICLES_SIZE 1
#define ARG_PARTICLES_COUNT 2

#define ARG_SPECTRUM_COLOR_COUNT 1
#define ARG_SPECTRUM_MODES 2

#define TIME_OFF 0
#define TIME_PARTICLE_SPEED 0
#define TIME_FADEIN 1
#define TIME_PARTICLE_DELAY 1
#define TIME_ON 2
#define TIME_FADEOUT 3
#define TIME_ROTATION 4
#define TIME_DELAY 5

#define rgb(r, g, b) r, g, b
#define is_black(buf) !(buf[0] || buf[1] || buf[2])
#define _color_brightness(brightness, r, g, b) \
scale8(r, brightness), scale8(g, brightness), scale8(b, brightness)
#define color_brightness(brightness, ...) _color_brightness(brightness, __VA_ARGS__)
#define color_from_buf(buf) (buf)[0], (buf)[1], (buf)[2]

#define set_color_manual(buf, ...) _set_color_manual(buf, __VA_ARGS__)
#define _set_color_manual(buf, r, g, b)\
(buf)[0] = r;\
(buf)[1] = g;\
(buf)[2] = b

#define set_color_manuali(buf, index, ...) _set_color_manuali(buf, index, __VA_ARGS__)
#define _set_color_manuali(buf, index, r, g, b)\
(buf)[index * 3 + 0] = r;\
(buf)[index * 3 + 1] = g;\
(buf)[index * 3 + 2] = b

#define set_color_manual_grb(buf, ...) _set_color_manual_grb(buf, __VA_ARGS__)
#define _set_color_manual_grb(buf, r, g, b)\
(buf)[0] = g;\
(buf)[1] = r;\
(buf)[2] = b

#define set_color_manuali_grb(buf, index, ...) _set_color_manuali_grb(buf,index, __VA_ARGS__)
#define _set_color_manuali_grb(buf, index, r, g, b)\
(buf)[index * 3+ 0] = g;\
(buf)[index * 3 + 1] = r;\
(buf)[index * 3 + 2] = b

#define grb(...) _grb(__VA_ARGS__)
#define _grb(r, g, b) g,r,b

#define COLOR_BLACK rgb(0, 0, 0)
#define COLOR_WHITE rgb(255, 255, 255)
#define COLOR_RED rgb(255, 0, 0)
#define COLOR_GREEN rgb(0, 255, 0)
#define COLOR_BLUE rgb(0, 0, 255)
#define COLOR_YELLOW rgb(255, 255, 0)
#define COLOR_MAGENTA rgb(255, 0, 255)
#define COLOR_CYAN rgb(0, 255, 255)

#if (LED_COUNT > 85)
typedef uint16_t led_index_t;
#else
typedef uint8_t led_index_t;
#endif /* (LED_COUNT > 85) */

#if LED_COUNT <= 255
typedef uint8_t led_count_t;
#else
typedef uint16_t led_count_t;
#endif

typedef enum {
    BREATHE = 0x00,
    FADE = 0x01,
    FILLING_FADE = 0x02,
    RAINBOW = 0x03,
    FILL = 0x04,
    ROTATING = 0x05,
    PARTICLES = 0x06,
    SPECTRUM = 0x07,
    PIECES = 0x0C,

} effect;

 uint8_t scale8(uint8_t i, uint8_t scale);

void set_all_colors(uint8_t *p_buf, uint8_t r, uint8_t g, uint8_t b, uint8_t count);

void set_color(uint8_t* p_buf, uint8_t led, uint8_t r, uint8_t g, uint8_t b);

void set_color_grb(uint8_t* p_buf, uint8_t led, uint8_t r, uint8_t g, uint8_t b);

void cross_fade(uint8_t* color, uint8_t* colors, uint8_t n_color, uint8_t m_color, uint8_t progress);

void rotate_buf(uint8_t* leds, uint8_t led_count, uint16_t rotation_progress, uint8_t start_led, uint16_t piece_leds,
                uint8_t bit_pack, uint8_t* colors, uint8_t color_count);

void simple_effect(effect effect, uint8_t* color, uint32_t frame, uint16_t* times, uint8_t* args, uint8_t* colors,
                   uint8_t color_count, uint8_t grb);

void digital_effect(effect effect, uint8_t* leds, uint8_t led_count, uint8_t start_led, uint32_t frame, uint16_t* times,
                    uint8_t* args, uint8_t* colors, uint8_t color_count);

uint8_t demo_music(uint8_t* fan_buf, uint8_t* pc_buf, uint8_t* gpu_buf, uint32_t frame);

uint8_t demo_effects(uint8_t* fan_buf, uint8_t* pc_buf, uint8_t* gpu_buf, uint32_t frame);

#endif //LEDCONTROLLER_COLOR_UTILS_H