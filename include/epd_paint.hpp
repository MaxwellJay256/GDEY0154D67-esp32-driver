/**
 * @file epd_paint.h
 * @brief GUI painter header file
 * @author @MaxwellJay256
 * @version 1.1
 */
#ifndef _EPD_PAINT_H_
#define _EPD_PAINT_H_

#include "epd_basic.h"
#include "fonts.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define ROTATE_0 0 // No rotation
#define ROTATE_90 90 // Rotate 90 degrees clockwise
#define ROTATE_180 180 // Rotate 180 degrees clockwise
#define ROTATE_270 270 // Rotate 270 degrees clockwise

/**
 * @brief The mirror image of the display, MIRROR_NONE, MIRROR_HORIZONTAL, MIRROR_VERTICAL, MIRROR_ORIGIN
 */
typedef enum {
    MIRROR_NONE = 0x00,
    MIRROR_HORIZONTAL = 0x01,
    MIRROR_VERTICAL = 0x02,
    MIRROR_ORIGIN = 0x03,
} MIRROR_IMAGE;
#define MIRROR_IMAGE_DEFAULT MIRROR_NONE // Default mirror image: none

#define IMAGE_BACKGROUND EPD_WHITE
#define FONT_FOREGROUND EPD_BLACK
#define FONT_BACKGROUND EPD_WHITE

/**
 * @brief The size of the point, from DOT_PIXEL_1X1 to DOT_PIXEL_8X8
 */
typedef enum {
    DOT_PIXEL_1X1 = 1, // 1 x 1
    DOT_PIXEL_2X2 , // 2 X 2
    DOT_PIXEL_3X3 ,	// 3 X 3
    DOT_PIXEL_4X4 ,	// 4 X 4
    DOT_PIXEL_5X5 , // 5 X 5
    DOT_PIXEL_6X6 , // 6 X 6
    DOT_PIXEL_7X7 , // 7 X 7
    DOT_PIXEL_8X8 , // 8 X 8
} DOT_PIXEL;
#define DOT_PIXEL_DEFAULT DOT_PIXEL_1X1  //Default dot pilex

/**
 * @brief The fill style of the point, DOT_FILL_AROUND or DOT_FILL_RIGHTUP
 */
typedef enum {
    DOT_FILL_AROUND = 1,		// dot pixel 1 x 1
    DOT_FILL_RIGHTUP , 		// dot pixel 2 X 2
} DOT_STYLE;
#define DOT_STYLE_DEFAULT DOT_FILL_AROUND  //Default dot pilex

/**
 * @brief Line style, LINE_STYLE_SOLID or LINE_STYLE_DOTTED
**/
typedef enum {
    LINE_STYLE_SOLID = 0,
    LINE_STYLE_DOTTED,
} LINE_STYLE;

/**
 * @brief Whether the graphic is filled, DRAW_FILL_EMPTY or DRAW_FILL_FULL
**/
typedef enum {
    DRAW_FILL_EMPTY = 0,
    DRAW_FILL_FULL,
} DRAW_FILL;

typedef struct {
    uint16_t x_start;
    uint16_t y_start;
    uint16_t width;
    uint16_t height;
} WINDOW;

/**
 * @brief image class that you can draw on
 */
class Paint
{
private:
    uint8_t *_image; // Pointer to the image buffer
    uint16_t _width; // Width of the image
    uint16_t _height; // Height of the image
    uint16_t _width_memory;
    uint16_t _height_memory;
    uint16_t _color;
    uint16_t _rotate;
    uint16_t _mirror;
    uint16_t _width_byte;
    uint16_t _height_byte;
    uint16_t _scale;

    void set_RAM_address(
        uint16_t x_start, uint16_t x_end, 
        uint16_t y_start1, uint16_t y_end1, 
        uint16_t y_start2, uint16_t y_end2);

public:
    Paint();
    Paint(uint8_t *image, uint16_t width, uint16_t height, uint16_t rotate, uint8_t color);
    ~Paint();

    void clear(uint8_t color=IMAGE_BACKGROUND);
    void clear_area(WINDOW window, uint8_t color=IMAGE_BACKGROUND);
    void print_full();
    // void print_fast(); // bug unfixed yet
    void print_part(WINDOW window);

    void set_image(uint8_t *image);
    void set_rotate(uint16_t rotate);
    void set_mirroring(uint16_t mirror);
    void set_scale(uint16_t scale);

    void draw_pixel(uint16_t x, uint16_t y, uint16_t color);
    void draw_point(uint16_t x, uint16_t y, uint16_t color, DOT_PIXEL dot_pixel, DOT_STYLE dot_style);
    void draw_line(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color, DOT_PIXEL line_width, LINE_STYLE line_style);
    void draw_rectangle(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color, DOT_PIXEL line_width, DRAW_FILL draw_fill);
    void draw_circle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color, DOT_PIXEL line_width, DRAW_FILL draw_fill);

    void draw_char(uint16_t x, uint16_t y, const char ascii_char, sFONT* font, uint16_t color=FONT_FOREGROUND, uint16_t background_color=FONT_BACKGROUND);
    void draw_string(uint16_t x, uint16_t y, const char *text, sFONT* font, uint16_t color=FONT_FOREGROUND, uint16_t background_color=FONT_BACKGROUND);
    void draw_num(uint16_t x, uint16_t y, int32_t num, sFONT* font, uint16_t color=FONT_FOREGROUND, uint16_t background_color=FONT_BACKGROUND);

    void draw_bitmap(const unsigned char *image_buffer);
    void draw_image(const unsigned char *image_buffer, uint16_t x_start, uint16_t y_start, uint16_t width, uint16_t height);
};

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // _EPD_PAINT_H_