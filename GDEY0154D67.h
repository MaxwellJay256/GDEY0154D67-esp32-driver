/**
 * @file GDEY0154D67.h
 * @brief GDEY0154D67 driver header file
 * @author @MaxwellJay256
 * @version 0.1
 */
#ifndef _GDEY0154D67_H_
#define _GDEY0154D67_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "GDEY0154D67_spi.h"

// IO settings
#define EPD_BUSY GPIO_NUM_13 // Busy pin, 1-busy 0-idle
#define EPD_RES GPIO_NUM_12  // Reset pin, 1-normal 0-reset
#define EPD_DC GPIO_NUM_14   // Data/Command, 1-data 0-command
#define EPD_CS GPIO_NUM_27   // Chip Select, 1-inactive 0-active
// SPI settings
#define EPD_SPI_MISO GPIO_NUM_23 // MISO signal
#define EPD_SPI_MOSI GPIO_NUM_26 // MOSI signal
#define EPD_SPI_CLK GPIO_NUM_25  // CLK signal

#define EPD_SCREEN_WIDTH 200  // Width of epaper
#define EPD_SCREEN_HEIGHT 200 // Height of epaper
#define EPD_DATA_LEN EPD_SCREEN_WIDTH * EPD_SCREEN_HEIGHT / 8 // Total length of an image data array
#define EPD_WHITE 0xff // White pixel
#define EPD_BLACK 0x00 // Black pixel

///////Initialize functions///////
/**
 * @brief Initialize epaper, including gpio, spi and SSD1681
 */
void epd_init_all(void);

/**
 * @brief Initialize epaper gpio pins(apart from spi)
 */
void epd_gpio_init(void);

/**
 * @brief Initialize epaper IC - SSD1681
*/
void epd_ssd1681_init(void);

///////Basic display functions///////
/**
 * @brief Clear the screen with black / white
 * @param color EPD_WHITE-white, EPD_BLACK-black
 */
void epd_clear_screen(uint8_t color);

/**
 * @brief Enter deep sleep mode
*/
void epd_deep_sleep(void);

///////Full display functions///////
/**
 * @brief Full refresh with a given data buffer
 * @param image_buffer Image buffer
 */
void epd_print_full_bydata(const uint8_t *data);

/**
 * @brief Full refresh with an image_display function(without parameters)
 * @param image_display Function to display image
 */
void epd_print_full_byfunction(void image_display(void));

/**
 * @brief Full refresh with an image_display function(with data)
 * @param display_func Function to display image
 * @param data Data array to display
 */
void epd_print_full(void display_func(const uint8_t *data), const uint8_t *data);

///////Partial display functions///////
/**
 * @brief Partial refresh with an image_display function (without parameters)
 * @param x_start X start position
 * @param y_start Y start position
 * @param x_size X size
 * @param y_size Y size
 * @param image_display Function to display image
 */
void epd_print_part_byfunction(
    uint16_t x_start, uint16_t y_start,
    uint16_t x_size, uint16_t y_size,
    void image_display(void));

/**
 * @brief Partial refresh with an image_display function (with data)
 * @param x_start X start position
 * @param y_start Y start position
 * @param x_size X size
 * @param y_size Y size
 * @param display_func Function to display image
 * @param data Data to display
 */
void epd_print_part(
    uint16_t x_start, uint16_t y_start,
    uint16_t x_size, uint16_t y_size,
    void display_func(const uint8_t *data), const uint8_t *data);

#endif // _GDEY0154D67_H_