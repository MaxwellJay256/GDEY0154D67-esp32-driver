/**
 * @file epd_commands.h
 * @brief GDEY0154D67 hex command definitions
 * @ref /docs/GDEW0154Z04_Specification.pdf
 */
#ifndef _EPD_COMMANDS_H_
#define _EPD_COMMANDS_H_

/// 0x00-0x0F
#define EPD_DRIVER_OUTPUT_CONTROL                   0x01 // Gate setting
#define EPD_GATE_DRIVING_VOLTAGE_CONTROL            0x03 // Set gate driving voltage
#define EPD_SOURCE_DRIVING_VOLTAGE_CONTROL          0x04 // Set source driving voltage
#define EPD_WRITE_REGISTER_FOR_INITIAL_CODE_SETTING 0x09 // Write register for initial code setting
#define EPD_READ_REGISTER_FOR_INITIAL_CODE_SETTING  0x0A // rRad register for initial code setting
#define EPD_BOOSTER_SOFT_START_CONTROL              0x0C // Set booster soft start
/// 0x10-0x1F
#define EPD_DEEP_SLEEP_MODE            0x10 // Deep Sleep Mode control
#define EPD_DATA_ENTRY_MODE_SETTING    0x11 // Define data entry sequence
#define EPD_SW_RESET                   0x12 // Resets the commands and parameters to their S/W reset default values, except Deep Sleep Mode (0x10)
#define EPD_TEMPERATURE_SENSOR_CONTROL 0x18 // Temperature sensor selection
#define EPD_TEMPERATURE_SENSOR_WRITE   0x1A // Write to temperature register
#define EPD_TEMPERATURE_SENSOR_READ    0x1B // Read from temperature register
/// 0x20-0x2F
#define EPD_MASTER_ACTIVATION         0x20 // Activate display update sequence
#define EPD_DISPLAY_UPDATE_COINTROL_1 0x21 // RAM content option for display update
#define EPD_DISPLAY_UPDATE_COINTROL_2 0x22 // Display update sequence option
#define EPD_WRITE_RAM                 0x24 // Write data into BW RAM
#define EPD_WRITE_RAM_RED             0x26 // Write data into RED RAM
#define EPD_READ_RAM                  0x27 // Read RAM data from display
/// 0x30-0x3F
#define EPD_BORDER_WAVEFORM_CONTROL 0x3C // Select border waveform for VBD
/// 0x40-0x4F
#define EPD_READ_RAM_OPTION                      0x41 // Read RAM option
#define EPD_SET_RAM_X_ADDRESS_START_END_POSITION 0x44 // Specify the start/end positions of the window address in the X direction
#define EPD_SET_RAM_Y_ADDRESS_START_END_POSITION 0x45 // Specify the start/end positions of the window address in the Y direction
#define EPD_AUTO_WRITE_RED_RAM                   0x46 // Auto Write RED RAM for Regular Pattern
#define EPD_AUTO_WRITE_BW_RAM                    0x47 // Auto Write B/W RAM for Regular Pattern
#define EPD_SET_RAM_X_ADDRESS_COUNTER            0x4E // Make initial settings for the RAM X address in the address counter (AC)
#define EPD_SET_RAM_Y_ADDRESS_COUNTER            0x4F // Make initial settings for the RAM Y address in the address counter (AC)

#define EPD_NOP 0X7F // Empty command, can terminate frame memory read/write

#endif // _EPD_COMMANDS_H_