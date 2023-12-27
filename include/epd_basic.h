/**
 * @file epd_basic.h
 * @brief GDEY0154D67 basic driver header file
 * @author @MaxwellJay256
 * @version 1.0
 */
#ifndef _EPD_BASIC_H_
#define _EPD_BASIC_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_spi.h"

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

void epd_init_all(void);
void epd_gpio_init(void);
void epd_IC_init(void);

void epd_wait_idle(void);
void epd_clear_screen(uint8_t color);
void epd_deep_sleep(void);

void epd_refresh_full(void);
void epd_refresh_part(void);
void epd_refresh_fast(void);

void epd_print_full_bydata(const uint8_t *data);
void epd_print_full_byfunction(void image_display(void));
void epd_print_full(void display_func(const uint8_t *data), const uint8_t *data);

///////Partial display functions///////
void epd_print_part_byfunction(
    uint16_t x_start, uint16_t y_start,
    uint16_t x_size, uint16_t y_size,
    void image_display(void));
void epd_print_part(
    uint16_t x_start, uint16_t y_start,
    uint16_t x_size, uint16_t y_size,
    void display_func(const uint8_t *data), const uint8_t *data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _EPD_BASIC_H_