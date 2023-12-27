/**
 * @file epd_basic.c
 * @brief GDEY0154D67 basic driver source file
 * @author @MaxwellJay256
 * @version 1.0
 */
#include "epd_basic.h"
#include "epd_commands.h"

static const char *TAG = "GDEY0154D67";

static bool epd_is_busy(void);
void epd_wait_idle(void);
static void epd_wait_timeout(int timeout);

/**
 * @brief Initialize epaper, including gpio, spi and SSD1681
 */
void epd_init_all(void)
{
    epd_gpio_init();
    epd_spi_init();
    epd_IC_init();
}

/**
 * @brief Initialize epaper gpio pins (apart from spi)
 */
void epd_gpio_init(void)
{
    ESP_LOGI(TAG, "Initializing GPIO pins...");
    // CS, DC, and RST pins
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT;       // set as output mode
    io_conf.pin_bit_mask =
        (1ULL << EPD_CS) | (1ULL << EPD_DC) | (1ULL << EPD_RES);
    io_conf.pull_down_en = 0; // disable pull-down mode
    io_conf.pull_up_en = 0;   // disable pull-up mode
    gpio_config(&io_conf);

    // Busy pin
    io_conf.intr_type = GPIO_INTR_NEGEDGE;     // interrupt on negative edge
    io_conf.mode = GPIO_MODE_INPUT;            // set as input mode
    io_conf.pin_bit_mask = (1ULL << EPD_BUSY); // apply to busy pin
    io_conf.pull_up_en = 1;                    // enable pull-up mode
    gpio_config(&io_conf);

    gpio_set_level(EPD_CS, 0);
    ESP_LOGI(TAG, "GPIO pins initialized.");
}

/**
 * @brief Initialize epaper IC - SSD1681
 */
void epd_IC_init(void)
{
    ESP_LOGI(TAG, "Initializing SSD1681...");

    gpio_set_level(EPD_RES, 0); // Reset module
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EPD_RES, 1); // Release reset
    vTaskDelay(pdMS_TO_TICKS(100));

    epd_wait_idle();
    epd_spi_send_command(EPD_SW_RESET);
    epd_wait_idle();

    epd_spi_send_command(EPD_DRIVER_OUTPUT_CONTROL); // Driver output control
    epd_spi_send_data(0xC7);    // Gate scan direction: GS0 -> GS63
    epd_spi_send_data(0x00);    // Source shift direction: S0 -> S199
    epd_spi_send_data(0x01);    // Booster switch: on

    epd_spi_send_command(EPD_DATA_ENTRY_MODE_SETTING);
    epd_spi_send_data(0x01);    // X increment, Y increment

    epd_spi_send_command(EPD_SET_RAM_X_ADDRESS_START_END_POSITION);
    epd_spi_send_data(0x00);    // RAM x address start at 00h;
    epd_spi_send_data(0x18);    // RAM x address end at 18h(24+240-1=263);

    epd_spi_send_command(EPD_SET_RAM_Y_ADDRESS_START_END_POSITION);
    epd_spi_send_data(0xC7);    // RAM y address start at C7h(199, b00011001);
    epd_spi_send_data(0x00);
    epd_spi_send_data(0x00);    // RAM y address end at 00h;
    epd_spi_send_data(0x00);

    epd_spi_send_command(EPD_BORDER_WAVEFORM_CONTROL);
    epd_spi_send_data(0x05);    // Border floating

    epd_spi_send_command(EPD_TEMPERATURE_SENSOR_CONTROL);
    epd_spi_send_data(0x80);    // Internal temperature sensor

    epd_spi_send_command(EPD_SET_RAM_X_ADDRESS_COUNTER);
    epd_spi_send_data(0x00);    // Set RAM x address count to 0;
    epd_spi_send_command(EPD_SET_RAM_Y_ADDRESS_COUNTER); 
    epd_spi_send_data(0xC7);    // Set RAM y address count to 0X199;
    epd_spi_send_data(0x00);
    epd_wait_idle();

    ESP_LOGI(TAG, "SSD1681 initialized.");
}

/**
 * @brief Check if epaper is busy
 * @return 
 *     - true - busy; false - idle
 */
static bool epd_is_busy(void)
{
    return gpio_get_level(EPD_BUSY);
}

/**
 * @brief Wait until epaper is idle
 */
void epd_wait_idle(void)
{
    while (1)
    {
        if (epd_is_busy() == false)
            break;
    }
}

/**
 * @brief Wait until epaper is idle or timeout
 * @param timeout Timeout in ms
 */
static void epd_wait_timeout(int timeout)
{
    int time_remain = timeout;
    while (time_remain > 0)
    {
        if (epd_is_busy() == false)
            break;
        vTaskDelay(pdMS_TO_TICKS(50));
        time_remain -= 50;
    }
    if (time_remain <= 0)
    {
        ESP_LOGE(TAG, "Timeout after %dms.", timeout);
    }
}

/**
 * @brief Enter deep sleep mode
 */
void epd_deep_sleep(void)
{
    ESP_LOGD(TAG, "Entering deep sleep mode...");
    epd_spi_send_command(EPD_DEEP_SLEEP_MODE);
    epd_spi_send_data(0x01); // Enter deep sleep mode 1
    vTaskDelay(pdMS_TO_TICKS(100));
}

/**
 * @brief Refresh the screen using full update mode
 */
void epd_refresh_full(void)
{
    ESP_LOGD(TAG, "Refreshing(full)...");
    epd_spi_send_command(EPD_DISPLAY_UPDATE_COINTROL_2); // Display update control 2
    epd_spi_send_data(0xF7);    // Load temperature and waveform setting
    epd_spi_send_command(EPD_MASTER_ACTIVATION);
    epd_wait_timeout(3000);     // Wait at most 1s
}
/**
 * @brief Refresh the screen using partial update mode
 */
void epd_refresh_part(void)
{
    ESP_LOGD(TAG, "Refreshing(partial)...");
    epd_spi_send_command(EPD_DISPLAY_UPDATE_COINTROL_2);
    epd_spi_send_data(0xFF);
    epd_spi_send_command(EPD_MASTER_ACTIVATION);
    epd_wait_timeout(1000);
}
/**
 * @brief Refresh the screen using fast update mode
 */
void epd_refresh_fast(void)
{
    ESP_LOGD(TAG, "Refreshing(fast)...");
    epd_spi_send_command(EPD_DISPLAY_UPDATE_COINTROL_2); // Display update control 2
    epd_spi_send_data(0xC7);    // C7: Without loading temperature value
    epd_spi_send_command(EPD_MASTER_ACTIVATION); // Activate display update sequence
    epd_wait_timeout(2000);      // Wait at most 100ms
}

/**
 * @brief Clear the screen with black / white
 * @param color EPD_WHITE-white, EPD_BLACK-black
 */
void epd_clear_screen(uint8_t color)
{
    ESP_LOGI(TAG, "Clearing screen with %s...", color ? "white" : "black");
    uint16_t i;
    epd_spi_send_command(EPD_WRITE_RAM);
    for (i = 0; i < EPD_DATA_LEN; ++i) {
        epd_spi_send_data(color);
    }
    epd_refresh_full();
    ESP_LOGI(TAG, "Screen cleared.");
}

/**
 * @brief Full refresh with a given data buffer
 * @param data Data to display
 */
void epd_print_full_bydata(const uint8_t *data)
{
    unsigned int i;
    epd_spi_send_command(EPD_WRITE_RAM); // Write RAM for black(0)/white (1)
    for (i = 0; i < EPD_DATA_LEN; ++i)
        epd_spi_send_data(*data++);
}

/**
 * @brief Full refresh with an image_display function(without parameters)
 * @param image_display Function to display image
 */
void epd_print_full_byfunction(void image_display(void))
{
    epd_IC_init();
    image_display(); // display image
    epd_refresh_full();
    epd_deep_sleep(); // enter deep sleep
}

/**
 * @brief Full refresh with an image_display function(with data)
 * @param display_func Function to display image
 * @param data Data to display
 */
void epd_print_full(
    void display_func(const uint8_t *data), const uint8_t *data)
{
    epd_IC_init();
    display_func(data); // display image
    epd_refresh_full();
    epd_deep_sleep(); // enter deep sleep
}

/**
 * @brief Set RAM address range (for partial refresh)
 * @note Used in epd_print_part_byfunction() and epd_print_part()
 * @param x_start X start position
 * @param y_start Y start position
 * @param x_size X size
 * @param y_size Y size
 */
static void epd_partial_set_RAM_address(
    uint16_t x_start, uint16_t y_start, uint16_t x_size, uint16_t y_size)
{
    uint16_t x_end, y_start1, y_start2, y_end1, y_end2;
    x_start = x_start / 8;
    x_end = x_start + x_size / 8 - 1;

    y_start1 = 0; // y_start1 should be the multiple of 8, since the basic element is 8 pixels.
    y_start2 = EPD_SCREEN_HEIGHT - y_start; // The last line of the image should be the last line of the last data byte
    if (y_start >= 256) {
        y_start1 = y_start2 / 256;
        y_start2 = y_start2 % 256;
    }
    y_end1 = 0;
    y_end2 = y_start2 + y_size - 1;
    if (y_end2 >= 256) {
        y_end1 = y_end2 / 256;
        y_end2 = y_end2 % 256;
    }

    epd_spi_send_command(EPD_SET_RAM_X_ADDRESS_START_END_POSITION); // Set RAM X start/end address
    epd_spi_send_data(x_start); // RAM x address start at 00h;
    epd_spi_send_data(x_end);   // RAM x address end at 18h(24+240-1=263);
    epd_spi_send_command(EPD_SET_RAM_Y_ADDRESS_START_END_POSITION); // Set RAM Y start/end address
    epd_spi_send_data(y_start2); // RAM y address start at C7h;
    epd_spi_send_data(y_start1); // RAM y address start at C7h;
    epd_spi_send_data(y_end2);   // RAM y address end at 00h(320-1=319);
    epd_spi_send_data(y_end1);   // RAM y address end at 00h(320-1=319);

    epd_spi_send_command(EPD_SET_RAM_X_ADDRESS_COUNTER); // Set RAM x address count to 0;
    epd_spi_send_data(x_start);
    epd_spi_send_command(EPD_SET_RAM_Y_ADDRESS_COUNTER); // Set RAM y address count to 0X127;
    epd_spi_send_data(y_start2);
    epd_spi_send_data(y_start1);
}

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
    void image_display(void))
{
    ESP_LOGD(TAG, "Partial refresh at x_start=%d, x_end=%d, y_start=%d, y_end=%d",
             x_start, x_start + x_size - 1, y_start, y_start + y_size - 1);
    // Add hardware reset to prevent background color change
    gpio_set_level(EPD_RES, 0); // Reset module
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EPD_RES, 1); // Release reset
    vTaskDelay(pdMS_TO_TICKS(10));

    // Lock the border to prevent accidental refresh
    epd_spi_send_command(EPD_BORDER_WAVEFORM_CONTROL); // Border waveform
    epd_spi_send_data(0x80);

    epd_partial_set_RAM_address(x_start, y_start, x_size, y_size);

    epd_spi_send_command(EPD_WRITE_RAM); // Write RAM for black(0)/white (1)
    image_display();

    epd_refresh_part();
    epd_deep_sleep();
}

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
    void display_func(const uint8_t *data), const uint8_t *data)
{
    ESP_LOGD(TAG, "Partial refresh at x_start=%d, x_end=%d, y_start=%d, y_end=%d",
             x_start, x_start + x_size - 1, y_start, y_start + y_size - 1);
    // Add hardware reset to prevent background color change
    gpio_set_level(EPD_RES, 0); // Reset module
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EPD_RES, 1); // Release reset
    vTaskDelay(pdMS_TO_TICKS(10));

    // Lock the border to prevent accidental refresh
    epd_spi_send_command(EPD_BORDER_WAVEFORM_CONTROL); // Border waveform
    epd_spi_send_data(0x80);

    epd_partial_set_RAM_address(x_start, y_start, x_size, y_size);

    epd_spi_send_command(EPD_WRITE_RAM); // Write RAM for black(0)/white (1)
    display_func(data);

    epd_refresh_part();
    epd_deep_sleep();
}

/*Experimental functions, not available for use!*/

/// @brief 2-dimension array to store partial data
uint8_t partial_data[EPD_SCREEN_HEIGHT][EPD_SCREEN_WIDTH / 8];

/// @brief 1-dimension array to store partial data
uint8_t partial_data_array[EPD_SCREEN_HEIGHT * EPD_SCREEN_WIDTH / 8];

/**
 * @brief Initialize partia_data and partial_data_array with all 0xff(white)
 */
void epd_partial_data_init(void)
{
    memset(partial_data, EPD_WHITE, sizeof(partial_data));
    memset(partial_data_array, EPD_WHITE, sizeof(partial_data_array));
}

/**
 * @brief Add partial data to partial_data_array
 * @param x_start X start position
 * @param x_end X end position
 * @param y_start Y start position
 * @param y_end Y end position
 * @param data Data to add
 */
void epd_partial_data_add(
    uint16_t x_start, uint16_t x_end,
    uint16_t y_start, uint16_t y_end,
    const uint8_t *data)
{
    uint8_t x_len = x_end - x_start;
    uint8_t x_data_location = x_start / 8;
    uint8_t x_size = x_len / 8;

    int data_index = 0;
    if (x_start % 8 != 0) {
        x_data_location++;
    }
    if (x_len % 8 != 0) {
        x_size++;
    }

    for (int x_index = y_start; x_index < y_end; ++x_index) {
        for (int y_index = x_data_location; y_index < (x_data_location + x_size); ++y_index) {
            partial_data[x_index][y_index] = (~data[data_index++]); // ~: black->white, white->black
        }
    }
}

/**
 * @brief Pass partial_data_array to epaper
 */
static void epd_print_partial_data(void)
{
    uint16_t i;
    epd_spi_send_command(EPD_WRITE_RAM);
    for (i = 0; i < EPD_DATA_LEN; ++i) {
        epd_spi_send_data(partial_data_array[i]);
    }

    epd_spi_send_command(EPD_WRITE_RAM_RED);
    for (i = 0; i < EPD_DATA_LEN; ++i) {
        epd_spi_send_data(partial_data_array[i]);
    }
}

/**
 * @brief Copy partial_data to partial_data_array
 */
void epd_partial_data_copy(void)
{
    int data_index = 0;
    for (int i = 0; i < EPD_SCREEN_HEIGHT; i++) {
        for (int j = 0; j < EPD_SCREEN_WIDTH / 8; j++) {
            partial_data_array[data_index++] = partial_data[i][j];
        }
    }
    epd_print_full_byfunction(epd_print_partial_data);
}

/**
 * @brief Set RAM value for base map
 * @param image_buffer Image buffer
 */
void epd_setRAMvalue_BaseMap(const uint8_t *image_buffer)
{
    uint16_t i;
    const uint8_t *data_flag;
    data_flag = image_buffer;

    epd_spi_send_command(0x24); // Write RAM for black(0)/white (1)
    for (i = 0; i < EPD_DATA_LEN; ++i)
        epd_spi_send_data(*image_buffer++);
    image_buffer = data_flag;

    epd_spi_send_command(0x26); // Write RAM for black(0)/white (1)
    for (i = 0; i < EPD_DATA_LEN; ++i)
        epd_spi_send_data(*image_buffer++);

    epd_refresh_full();
}