/**
 * @file epd_paint.cpp
 * @brief GUI painter source file
 * @author @MaxwellJay256
 * @version 1.1
 */
#include "epd_paint.hpp"
#include "epd_commands.h"

static const char *TAG = "GDEY0154D67-Paint";

/**
 * @brief Default constructor, no image is set
 */
Paint::Paint() :
    _width(EPD_SCREEN_WIDTH), _height(EPD_SCREEN_HEIGHT),
    _color(EPD_BLACK),
    _rotate(ROTATE_0),
    _mirror(MIRROR_NONE),
    _scale(2)
{
    _image = new uint8_t[EPD_DATA_LEN];
    _width_byte = (_width % 8 == 0)? (_width / 8 ): (_width / 8 + 1);
    _height_byte = _height;
    _width_memory = _width_byte;
    _height_memory = _height;
    ESP_LOGI(TAG, "Paint object created with default parameters.");
}

/**
 * @brief Constructor with all parameters
 * @param image Pointer to the image buffer (you can use "new uint8_t[EPD_DATA_LEN]")
 * @param width Width of the image
 * @param height Height of the image
 * @param rotate Rotation of the image
 * @param color Color of the image
 */
Paint::Paint(uint8_t *image, uint16_t width, uint16_t height, uint16_t rotate, uint8_t color) :
    _image(image),
    _width(width), _height(height),
    _color(color),
    _rotate(rotate),
    _mirror(MIRROR_NONE),
    _scale(2)
{
    _width_byte = (_width % 8 == 0)? (_width / 8 ): (_width / 8 + 1);
    _height_byte = _height;
    _width_memory = _width_byte;
    _height_memory = _height;
    ESP_LOGI(TAG, "Paint object created with parameters.");
}

Paint::~Paint()
{
    ESP_LOGD(TAG, "Paint object destroyed.");
}

/**
 * @brief Clear the canvas
 * @param color Color to fill (EPD_WHITE or EPD_BLACK)
 */
void Paint::clear(uint8_t color)
{
    // Write color to every byte of _image
    for (uint16_t j = 0; j < _height_byte; j++) {
        for (uint16_t i = 0; i < _width_byte; i++) {
            _image[i + j * _width_byte] = color;
        }
    }
    ESP_LOGI(TAG, "Canvas cleared.");
}

/**
 * @brief Clear the canvas in a specific area
 * @param window Area to clear
 * @param color Color to fill (EPD_WHITE or EPD_BLACK)
 */
void Paint::clear_area(WINDOW window, uint8_t color)
{
    // Write color to every byte of _image
    for (uint16_t j = window.y_start; j < window.y_start + window.height; j++) {
        for (uint16_t i = window.x_start / 8; i < window.x_start / 8 + window.width / 8; i++) {
            _image[i + j * _width_byte] = color;
        }
    }
    ESP_LOGI(TAG, "Canvas cleared in area (%d, %d) - (%d, %d).", window.x_start, window.y_start, window.x_start + window.width, window.y_start + window.height);
}

void Paint::set_RAM_address(
    uint16_t x_start, uint16_t x_end, 
    uint16_t y_start1, uint16_t y_end1, 
    uint16_t y_start2, uint16_t y_end2)
{
    epd_spi_send_command(EPD_SET_RAM_X_ADDRESS_START_END_POSITION);
    epd_spi_send_data(x_start);
    epd_spi_send_data(x_end);
    epd_spi_send_command(EPD_SET_RAM_Y_ADDRESS_START_END_POSITION);
    epd_spi_send_data(y_start2);
    epd_spi_send_data(y_start1);
    epd_spi_send_data(y_end2);
    epd_spi_send_data(y_end1);

    epd_spi_send_command(EPD_SET_RAM_X_ADDRESS_COUNTER);
    epd_spi_send_data(x_start);
    epd_spi_send_command(EPD_SET_RAM_Y_ADDRESS_COUNTER);
    epd_spi_send_data(y_start2);
    epd_spi_send_data(y_start1);
}

/**
 * @brief Print the image using full refresh
 */
void Paint::print_full()
{
    if (_image == NULL) {
        ESP_LOGE(TAG, "Image is not set.");
        return;
    }

    ESP_LOGI(TAG, "Printing canvas with full refresh...");
    
    gpio_set_level(EPD_RES, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EPD_RES, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    epd_spi_send_command(EPD_BORDER_WAVEFORM_CONTROL);
    epd_spi_send_data(0x05);

    set_RAM_address(0x00, 0x18, 0x00, 0x00, 0xC7, 0x00);

    epd_spi_send_command(EPD_WRITE_RAM);
    for (uint16_t j = 0; j < _height_byte; j++) {
        for (uint16_t i = 0; i < _width_byte; i++) {
            epd_spi_send_data(_image[i + j * _width_byte]);
        }
    }

    epd_refresh_full();
}

/**
 * @brief Print the image using partial refresh
 */
void Paint::print_part(WINDOW window)
{
    if (_image == NULL) {
        ESP_LOGE(TAG, "Image is not set.");
        return;
    }

    ESP_LOGI(TAG, "Printing canvas with partial refresh...");
    unsigned int x_start = window.x_start / 8;
    unsigned int x_end= window.width / 8 + x_start - 1;

    unsigned int y_start1 = 0; // y_start 高 8 位
    unsigned int y_start2 = EPD_SCREEN_HEIGHT - window.y_start; // y_start 低 8 位
    unsigned int y_end1 = 0; // y_end 高 8 位
    unsigned int y_end2 = y_start2 + window.height - 1; // y_end 低 8 位
    
    // 将 y_start 拆分成两个字节
    if (y_start2 >= 256) {
        y_start1 = y_start2 / 256;
        y_start2 = y_start2 % 256;
    }
    // 将 y_end 拆分成两个字节
    if (y_end2 >= 256) {
        y_end1 = y_end2 / 256;
        y_end2 = y_end2 % 256;
    }
    
    gpio_set_level(EPD_RES, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EPD_RES, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    epd_spi_send_command(EPD_BORDER_WAVEFORM_CONTROL);
    epd_spi_send_data(0x80);

    set_RAM_address(x_start, x_end, y_start1, y_end1, y_start2, y_end2);

    epd_spi_send_command(EPD_WRITE_RAM);
    for (uint16_t j = window.y_start; j < window.y_start + window.height; ++j) {
        for (uint16_t i = window.x_start / 8; i < (window.x_start + window.width) / 8; ++i) {
            epd_spi_send_data(_image[i + j * _width_byte]);
        }
    }

    epd_refresh_part();
}

/*/
void Paint::print_fast()
{
    if (_image == NULL) {
        ESP_LOGE(TAG, "Image is not set.");
        return;
    }

    ESP_LOGI(TAG, "Printing canvas with fast refresh...");
    
    gpio_set_level(EPD_RES, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EPD_RES, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    epd_spi_send_command(EPD_SW_RESET);
    epd_wait_idle();

    epd_spi_send_command(EPD_TEMPERATURE_SENSOR_CONTROL);
    epd_spi_send_data(0x80);

    epd_spi_send_command(EPD_DISPLAY_UPDATE_COINTROL_2);
    epd_spi_send_data(0xB1);
    epd_spi_send_command(EPD_MASTER_ACTIVATION);
    epd_wait_idle();

    epd_spi_send_command(EPD_TEMPERATURE_SENSOR_WRITE);
    epd_spi_send_data(0x64);
    epd_spi_send_data(0x00);

    epd_spi_send_command(EPD_DISPLAY_UPDATE_COINTROL_2);
    epd_spi_send_data(0x91);
    epd_spi_send_command(EPD_MASTER_ACTIVATION);
    epd_wait_idle();

    set_rotate(ROTATE_180);
    epd_spi_send_command(EPD_WRITE_RAM);
    for (uint16_t j = 0; j < _height_byte; j++) {
        for (uint16_t i = 0; i < _width_byte; i++) {
            epd_spi_send_data(_image[i + j * _width_byte]);
        }
    }

    epd_refresh_fast();
    set_rotate(ROTATE_0);
}
//*/

/**
 * @brief Set the image buffer
 * @param image Pointer to the image buffer
 */
void Paint::set_image(uint8_t *image)
{
    _image = image;
}

/**
 * @brief Set the rotation of the image
 * @param ratate ROTATE_0-0, ROTATE_90-90, ROTATE_180-180, ROTATE_270-270
 */
void Paint::set_rotate(uint16_t rotate)
{
    if (rotate == ROTATE_0 || rotate == ROTATE_90 || rotate == ROTATE_180 || rotate == ROTATE_270) {
        _rotate = rotate;
        ESP_LOGD(TAG, "Rotation set to %d.", rotate);
    } else {
        ESP_LOGW(TAG, "Rotation must be 0, 90, 180 or 270.");
    }
}

/**
 * @brief Set the mirroring of the image
 * @param mirror MIRROR_NONE, MIRROR_HORIZONTAL, MIRROR_VERTICAL or MIRROR_ORIGIN
 */
void Paint::set_mirroring(uint16_t mirror)
{
    if (mirror == MIRROR_NONE || mirror == MIRROR_HORIZONTAL ||
        mirror == MIRROR_VERTICAL || mirror == MIRROR_ORIGIN) {
        _mirror = mirror;
        ESP_LOGD(TAG, "Mirror set to %d.", mirror);
    } else {
        ESP_LOGW(TAG, "Mirror must be MIRROR_NONE, MIRROR_HORIZONTAL, \
            MIRROR_VERTICAL or MIRROR_ORIGIN.");
    }
}

/**
 * @brief Set the scale of the image
 * @param scale 2, 4 or 7
 */
void Paint::set_scale(uint16_t scale)
{
    ESP_LOGD(TAG, "Setting scale to %d...", scale);
    if (scale == 2) {
        _scale = scale;
        _width_byte = (_width_memory % 8 == 0)? (_width_memory / 8 ): (_width_memory / 8 + 1);
    }
    else if (scale == 4) {
        _scale = scale;
        _width_byte = (_width_memory % 4 == 0)? (_width_memory / 4 ): (_width_memory / 4 + 1);
    }
    else if (scale == 7) {
        _scale = scale;
        _width_byte = (_width_memory % 2 == 0)? (_width_memory / 2 ): (_width_memory / 2 + 1);
    }
    else {
        ESP_LOGW(TAG, "Scale only supports 2, 4, and 7.");
    }
}

/**
 * @brief Draw a single pixel
 * @param x x coordinate
 * @param y y coordinate
 * @param color Color of the pixel
 */
void Paint::draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x > _width || y > _height) {
        ESP_LOGE(TAG, "Exceeding display boundaries.");
        return;
    }

    uint16_t point_x = x;
    uint16_t point_y = y;
    ESP_LOGV(TAG, "Drawing pixel at (%d, %d).", x, y);

    // Calculate the coordinates of the point after rotation
    switch (_rotate) {
        case ROTATE_0:

            break;
        case ROTATE_90:
            point_x = _height - y;
            point_y = x;
            break;
        case ROTATE_180:
            point_x = _width - x;
            point_y = _height - y;
            break;
        case ROTATE_270:
            point_x = y;
            point_y = _width - x;
            break;
        default:
            break;
    }

    // Calculate the coordinates of the point after mirroring
    switch (_mirror) {
        case MIRROR_NONE:
            break;
        case MIRROR_HORIZONTAL:
            point_x = _width - point_x;
            break;
        case MIRROR_VERTICAL:
            point_y = _height - point_y;
            break;
        case MIRROR_ORIGIN:
            point_x = _width - point_x;
            point_y = _height - point_y;
            break;
        default:
            break;
    }

    if (point_x > _width || point_y > _height) {
        ESP_LOGE(TAG, "Exceeding display boundaries.");
        return;
    }

    uint32_t addr = 0;
    uint8_t data = 0;

    switch (_scale)
    {
    case 2:
        addr = point_x / 8 + point_y * _width_byte;
        data = _image[addr];
        if (color == EPD_BLACK)
            _image[addr] = data & ~(0x80 >> (point_x % 8));
        else
            _image[addr] = data | (0x80 >> (point_x % 8));
        break;
    case 4:
        addr = point_x / 4 + point_y * _width_byte;
        data = _image[addr];

        data = data & (~(0xC0 >> ((point_x % 4) * 2)));
        _image[addr] = data | ((color << 6) >> ((point_x % 4) * 2));
        break;
    case 7:
        uint16_t width = _width_memory * 3 % 8 == 0 ? _width_memory * 3 / 8 : _width_memory * 3 / 8 + 1;
        addr = point_x / 8 * 3 + point_y * width;
        uint8_t shift = (point_x + point_y * _width_memory) % 8;
        uint8_t data_2 = 0;

        switch (shift)
        {
        case 0:
            data = _image[addr] & 0x1F;
            data = data | ((color << 5) | 0xE0);
            _image[addr] = data;
            break;
        case 1:
            data = _image[addr] & 0xE3;
            data = data | ((color << 2) | 0x1C);
            _image[addr] = data;
            break;
        case 2:
            data = _image[addr] & 0xFC; // 0b11111100
            data_2 = _image[addr + 1] & 0x7F; // 0b01111111
            data = data | ((color >> 1) & 0x03);
            data_2 = data_2 | ((color << 7) & 0x80);
            _image[addr] = data;
            _image[addr + 1] = data_2;
            break;
        case 3:
            data = _image[addr] & 0x8F;
            data = data | ((color << 4) | 0x70);
            _image[addr] = data;
            break;
        case 4:
            data = _image[addr] & 0xF1;
            data = data | ((color << 1) | 0x0E);
            _image[addr] = data;
            break;
        case 5:
            data = _image[addr] & 0xFE;
            data_2 = _image[addr + 1] & 0x3F;
            data = data | ((color >> 2) & 0x01);
            data_2 = data_2 | ((color << 6) & 0xC0);
            _image[addr] = data;
            _image[addr + 1] = data_2;
            break;
        case 6:
            data = _image[addr] & 0xC7;
            data = data | ((color << 3) | 0x38);
            _image[addr] = data;
            break;
        case 7:
            data = _image[addr] & 0xF8;
            data = data | (color & 0x07);
            _image[addr] = data;
            break;
        }
        break;
    }
}

/**
 * @brief Draw a dot with its center at (x, y)
 * @param x x coordinate
 * @param y y coordinate
 * @param color Color of the dot
 * @param dot_pixel Size of the dot
 * @param dot_style Style of the dot
 */
void Paint::draw_point(uint16_t x, uint16_t y, uint16_t color, DOT_PIXEL dot_pixel, DOT_STYLE dot_style)
{
    if (x > _width || y > _height) {
        ESP_LOGE(TAG, "Exceeding display boundaries.");
        return;
    }

    int16_t xDir_num, yDir_num; // Draw a dot direction, x, y-axis
    if (dot_style == DOT_FILL_AROUND)
    {
        for (xDir_num = 0; xDir_num < 2 * dot_pixel - 1; xDir_num++) {
            for (yDir_num = 0; yDir_num < 2 * dot_pixel - 1; yDir_num++) {
                if (x + xDir_num - dot_pixel < 0 || y + yDir_num - dot_pixel < 0) {
                    break;
                }
                draw_pixel(x + xDir_num - dot_pixel, y + yDir_num - dot_pixel, color);
            }
        }
    }
    else {
        for (xDir_num = 0; xDir_num <  dot_pixel; xDir_num++) {
            for (yDir_num = 0; yDir_num <  dot_pixel; yDir_num++) {
                draw_pixel(x + xDir_num - 1, y + yDir_num - 1, color);
            }
        }
    }
}

/**
 * @brief Draw a line from (x_start, y_start) to (x_end, y_end)
 * @param x_start x coordinate of the starting point
 * @param y_start y coordinate of the starting point
 * @param x_end x coordinate of the ending point
 * @param y_end y coordinate of the ending point
 * @param color Color of the line
 * @param line_width Width of the line
 * @param line_style Style of the line (LINE_STYLE_SOLID / LINE_STYLE_DOTTED)
 */
void Paint::draw_line(
    uint16_t x_start, uint16_t y_start, 
    uint16_t x_end, uint16_t y_end, 
    uint16_t color, DOT_PIXEL line_width, LINE_STYLE line_style)
{
    if (x_start > _width || y_start > _height || x_end > _width || y_end > _height) {
        ESP_LOGE(TAG, "Exceeding display boundaries.");
        return;
    }
    ESP_LOGD(TAG, "Drawing line from (%d, %d) to (%d, %d).", x_start, y_start, x_end, y_end);

    uint16_t x_point = x_start;
    uint16_t y_point = y_start;
    int dx = (int)x_end - (int)x_start >= 0 ? x_end - x_start : x_start - x_end;
    int dy = (int)y_end - (int)y_start <= 0 ? y_end - y_start : y_start - y_end;

    // Increment direction, 1 is positive, -1 is counter;
    int x_addway = x_start < x_end ? 1 : -1;
    int y_addway = y_start < y_end ? 1 : -1;

    //Cumulative error
    int esp = dx + dy;
    char Dotted_Len = 0; // Dotted line length

    for (;;) {
        Dotted_Len++;
        //Painted dotted line, 2 point is really virtual
        if (line_style == LINE_STYLE_DOTTED && Dotted_Len % 3 == 0) {
            //Debug("LINE_DOTTED\r\n");
            draw_point(x_point, y_point, EPD_WHITE, line_width, DOT_STYLE_DEFAULT);
            Dotted_Len = 0;
        } else {
            draw_point(x_point, y_point, color, line_width, DOT_STYLE_DEFAULT);
        }
        if (2 * esp >= dy) {
            if (x_point == x_end) break;
            esp += dy;
            x_point += x_addway;
        }
        if (2 * esp <= dx) {
            if (y_point == y_end) break;
            esp += dx;
            y_point += y_addway;
        }
    }
}

/**
 * @brief Draw a rectangle from (x_start, y_start) to (x_end, y_end)
 * @param x_start x coordinate of the starting point
 * @param y_start y coordinate of the starting point
 * @param x_end x coordinate of the ending point
 * @param y_end y coordinate of the ending point
 * @param color Color of the rectangle
 * @param line_width Width of the rectangle
 * @param draw_fill Whether to fill the rectangle (DRAW_FILL_EMPTY / DRAW_FILL_FULL)
 */
void Paint::draw_rectangle(
    uint16_t x_start, uint16_t y_start, 
    uint16_t x_end, uint16_t y_end, 
    uint16_t color, DOT_PIXEL line_width, DRAW_FILL draw_fill)
{
    if (x_start > _width || y_start > _height || x_end > _width || y_end > _height) {
        ESP_LOGE(TAG, "Exceeding display boundaries.");
        return;
    }
    ESP_LOGI(TAG, "Drawing rectangle from (%d, %d) to (%d, %d).", x_start, y_start, x_end, y_end);

    if (draw_fill) {
        uint16_t y;
        for (y = y_start; y < y_end; y++) {
            draw_line(x_start, y, x_end, y, color , line_width, LINE_STYLE_SOLID);
        }
    } else {
        draw_line(x_start, y_start, x_end, y_start, color, line_width, LINE_STYLE_SOLID);
        draw_line(x_start, y_start, x_start, y_end, color, line_width, LINE_STYLE_SOLID);
        draw_line(x_end, y_end, x_end, y_start, color, line_width, LINE_STYLE_SOLID);
        draw_line(x_end, y_end, x_start, y_end, color, line_width, LINE_STYLE_SOLID);
    }
}

/**
 * @brief Draw a circle with center at (x, y) and radius r
 * @param x x coordinate of the center
 * @param y y coordinate of the center
 * @param radius Radius of the circle
 * @param color Color of the circle
 * @param line_width Width of the circle
 * @param draw_fill Whether to fill the circle (DRAW_FILL_EMPTY / DRAW_FILL_FULL)
 */
void Paint::draw_circle(
    uint16_t x, uint16_t y, 
    uint16_t radius, uint16_t color, 
    DOT_PIXEL line_width, DRAW_FILL draw_fill)
{
    if (x > _width || y >= _height) {
        ESP_LOGE(TAG, "Exceeding display boundaries.");
        return;
    }
    ESP_LOGI(TAG, "Drawing circle at (%d, %d) with radius %d.", x, y, radius);

    // Draw a circle from(0, r) as a starting point
    int16_t x_current = 0, y_current = radius;

    // Cumulative error, judge the next point of the logo
    int16_t esp = 3 - (radius << 1);

    int16_t sCountY;
    if (draw_fill == DRAW_FILL_FULL)
    {
        while (x_current <= y_current)
        {
            for (sCountY = x_current; sCountY <= y_current; ++sCountY) {
                draw_point(x + x_current, y + sCountY, color, DOT_PIXEL_1X1, DOT_STYLE_DEFAULT); // 1
                draw_point(x - x_current, y + sCountY, color, DOT_PIXEL_1X1, DOT_STYLE_DEFAULT); // 2
                draw_point(x - sCountY, y + x_current, color, DOT_PIXEL_1X1, DOT_STYLE_DEFAULT); // 3
                draw_point(x - sCountY, y - x_current, color, DOT_PIXEL_1X1, DOT_STYLE_DEFAULT); // 4
                draw_point(x - x_current, y - sCountY, color, DOT_PIXEL_1X1, DOT_STYLE_DEFAULT); // 5
                draw_point(x + x_current, y - sCountY, color, DOT_PIXEL_1X1, DOT_STYLE_DEFAULT); // 6
                draw_point(x + sCountY, y - x_current, color, DOT_PIXEL_1X1, DOT_STYLE_DEFAULT); // 7
                draw_point(x + sCountY, y + x_current, color, DOT_PIXEL_1X1, DOT_STYLE_DEFAULT);
            }
            if (esp < 0)
                esp += 4 * x_current + 6;
            else {
                esp += 10 + 4 * (x_current - y_current);
                y_current--;
            }
            x_current++;
        }
    }
    else { // Draw a hollow circle
        while (x_current <= y_current)
        {
            draw_point(x + x_current, y + y_current, color, line_width, DOT_STYLE_DEFAULT); //1
            draw_point(x - x_current, y + y_current, color, line_width, DOT_STYLE_DEFAULT); //2
            draw_point(x - y_current, y + x_current, color, line_width, DOT_STYLE_DEFAULT); //3
            draw_point(x - y_current, y - x_current, color, line_width, DOT_STYLE_DEFAULT); //4
            draw_point(x - x_current, y - y_current, color, line_width, DOT_STYLE_DEFAULT); //5
            draw_point(x + x_current, y - y_current, color, line_width, DOT_STYLE_DEFAULT); //6
            draw_point(x + y_current, y - x_current, color, line_width, DOT_STYLE_DEFAULT); //7
            draw_point(x + y_current, y + x_current, color, line_width, DOT_STYLE_DEFAULT); //0

            if (esp < 0)
                esp += 4 * x_current + 6;
            else {
                esp += 10 + 4 * (x_current - y_current);
                y_current--;
            }
            x_current++;
        }
    }
}

/**
 * @brief Draw a single ASCII character at (x, y)
 * @param x x coordinate of the starting point
 * @param y y coordinate of the starting point
 * @param ascii_char ASCII character
 * @param font Font of the character
 * @param color Color of the character
 * @param background_color Background color of the character
 */
void Paint::draw_char(
    uint16_t x, uint16_t y, 
    const char ascii_char, sFONT* font, 
    uint16_t color, uint16_t background_color)
{
    uint16_t page, column;

    if (x > _width || y > _height) {
        ESP_LOGE(TAG, "Exceeding display boundaries.");
        return;
    }
    ESP_LOGD(TAG, "Drawing character '%c' at (%d, %d).", ascii_char, x, y);

    uint32_t char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const uint8_t* ptr = &font->table[char_offset];

    for (page = 0; page < font->Height; ++page) {
        for (column = 0; column < font->Width; ++column) {
            //To determine whether the font background color and screen background color is consistent
            if (FONT_BACKGROUND == background_color) { //this process is to speed up the scan
                if (*ptr & (0x80 >> (column % 8)))
                    draw_pixel(x + column, y + page, color);
            } else {
                if (*ptr & (0x80 >> (column % 8))) {
                    draw_pixel(x + column, y + page, color);
                } else {
                    draw_pixel(x + column, y + page, background_color);
                }
            }
            // One pixel is 8 bits
            if (column % 8 == 7) {
                ptr++;
            }
        } // Write a line
        if (font->Width % 8 != 0) {
            ptr++;
        }
    } // Write all
}

/**
 * @brief Draw a string at (x, y)
 * @param x x coordinate of the starting point
 * @param y y coordinate of the starting point
 * @param text String to be drawn
 * @param font Font of the string
 * @param color Color of the string
 * @param background_color Background color of the string
 */
void Paint::draw_string(
    uint16_t x, uint16_t y, 
    const char *text, sFONT* font, 
    uint16_t color, uint16_t background_color)
{
    if (x > _width || y > _height) {
        ESP_LOGE(TAG, "Input string exceeds the display boundaries.");
        return;
    }
    ESP_LOGI(TAG, "Drawing string '%s' at (%d, %d).", text, x, y);

    uint16_t x_point = x, y_point = y;
    while (*text != '\0')
    {
        //if x direction filled , reposition to(x,y),y increased
        if ((x_point + font->Width) > _width) {
            x_point = x;
            y_point += font->Height;
        }

        // If the y direction is full, reposition to (x, y)
        if ((y_point  + font->Height) > _height) {
            x_point = x;
            y_point = y;
        }
        draw_char(x_point, y_point, *text, font, color, background_color);
        //The next word of the address
        text ++;
        //The next word of the abscissa increases the font of the broadband
        x_point += font->Width;
    }
}

#define ARRAY_LEN 255
/**
 * @brief Draw a number at (x, y)
 * @param x x coordinate of the starting point
 * @param y y coordinate of the starting point
 * @param num Number to be drawn
 * @param font Font of the number
 * @param color Color of the number
 * @param background_color Background color of the number
 */
void Paint::draw_num(
    uint16_t x, uint16_t y, 
    int32_t num, sFONT* font, 
    uint16_t color, uint16_t background_color)
{
    if (x > _width || y > _height) {
        ESP_LOGE(TAG, "Exceeding display boundaries.");
        return;
    }
    ESP_LOGI(TAG, "Drawing number '%ld' at (%d, %d).", (long)num, x, y);

    uint8_t str[ARRAY_LEN] = {0};
    sprintf((char *)str, "%ld", (long)num);
    draw_string(x, y, (const char *)str, font, color, background_color);
}

/**
 * @brief Draw an image directly from a given buffer
 * @param image_buffer Pointer to the image buffer (size must be EPD_DATA_LEN)
 */
void Paint::draw_bitmap(const unsigned char *image_buffer)
{
    uint16_t x, y;
    uint32_t addr = 0;

    for (y = 0; y < _height_byte; ++y) {
        for (x = 0; x < _width_byte; ++x) {
            addr = x + y * _width_byte;
            _image[addr] = (unsigned char)image_buffer[addr];
        }
    }
}

/**
 * @brief Draw an image from a given buffer at (x_start, y_start)
 * @param image_buffer Pointer to the image buffer
 * @param x_start x coordinate of the starting point
 * @param y_start y coordinate of the starting point
 * @param width Width of the image
 * @param height Height of the image
 */
void Paint::draw_image(
    const unsigned char *image_buffer, 
    uint16_t x_start, uint16_t y_start, uint16_t width, uint16_t height)
{
    uint16_t x, y;
    uint16_t w_byte = width % 8 == 0 ? width / 8 : width / 8 + 1;
    uint32_t addr = 0; // The address of the point in the image_buffer
    uint32_t pAddr = 0; // The address of the point in the real picture

    ESP_LOGI(TAG, "Drawing image at (%d, %d) with width %d and height %d.", x_start, y_start, width, height);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < w_byte; ++x) {
            addr = x + y * w_byte;
            pAddr = (x + x_start) + (y + y_start) * _width_byte;
            _image[pAddr] = (unsigned char)image_buffer[addr];
        }
    }
}