#ifndef _GDEY0154D67_SPI_H_
#define _GDEY0154D67_SPI_H_

#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

/**
 * @brief Initialize epaper spi bus
 */
void epd_spi_init(void);

/**
 * @brief Send data to epaper spi bus
 * @param data Data to send
 */
void epd_spi_send_data(const uint8_t data);

/**
 * @brief Send command to epaper spi bus
 * @param command Command to send
 */
void epd_spi_send_command(const uint8_t cmd);

#endif // _GDEY0154D67_SPI_H_