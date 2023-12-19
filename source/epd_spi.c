/**
 * @file epd_spi.c
 * @brief GDEY0154D67 spi driver source file
 * @version 1.0
 */
#include "epd_basic.h"

static const char *TAG = "GDEY0154D67_spi";
static spi_device_handle_t spi;

void epd_spi_init(void)
{
    esp_err_t esp_err;
    spi_bus_config_t bus_config = {
        .miso_io_num = EPD_SPI_MISO, // MISO signal
        .mosi_io_num = EPD_SPI_MOSI, // MOSI signal
        .sclk_io_num = EPD_SPI_CLK,  // CLK
        .quadwp_io_num = -1,         // WP signal, special for D2 in QSPI mode
        .quadhd_io_num = -1,         // HD signal, special for D3 in QSPI mode
        .max_transfer_sz = 64 * 8,   // maximum transfer size, in bytes
    };
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 15 * 1000 * 1000, // clock speed
        .mode = 0,                          // spi mode 0
        .queue_size = 7,                    // queue 7 transactions at a time
        .pre_cb = NULL,                     // no pre-transfer callback
    };

    // Initialize the SPI bus
    ESP_LOGI(TAG, "Initializing SPI bus...");
    esp_err = spi_bus_initialize(HSPI_HOST, &bus_config, 0);
    ESP_ERROR_CHECK(esp_err);

    // Attach the device to the SPI bus
    ESP_LOGI(TAG, "Attaching device to SPI bus...");
    esp_err = spi_bus_add_device(HSPI_HOST, &device_config, &spi);
    ESP_ERROR_CHECK(esp_err);

    ESP_LOGI(TAG, "SPI bus initialized.");
}

void epd_spi_send_data(const uint8_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    gpio_set_level(EPD_DC, 1); // set DC pin to high
    gpio_set_level(EPD_CS, 0); // set CS pin to low

    memset(&t, 0, sizeof(t)); // zero out the transaction
    t.length = 8; // 8 bits
    t.tx_buffer = &data; // data to send
    t.user = (void *)1; // D/C needs to be set to 1

    ret = spi_device_transmit(spi, &t); // transmit!
    assert(ret == ESP_OK);

    gpio_set_level(EPD_CS, 1); // set CS pin to high
}

void epd_spi_send_command(const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    gpio_set_level(EPD_DC, 0); // set DC pin to low
    gpio_set_level(EPD_CS, 0); // set CS pin to low

    memset(&t, 0, sizeof(t)); // zero out the transaction
    t.length = 8; // 8 bits
    t.tx_buffer = &cmd; // command to send
    t.user = (void *)0; // D/C needs to be set to 0

    ret = spi_device_transmit(spi, &t); // transmit!
    assert(ret == ESP_OK);

    gpio_set_level(EPD_CS, 1); // set CS pin to high
}