/**
 * @file main.c
 * @brief This file contains the main implementation for interfacing with an OLED display using I2C on an AVR microcontroller.
 *
 * The code initializes the I2C interface and the OLED display, and then writes temperature and humidity values to the display.
 *
 * @details
 * - The I2C interface is initialized with a clock speed of 100kHz.
 * - The OLED display is initialized with a series of commands to set up the display parameters.
 * - A simple 5x7 font is defined in program memory for displaying characters on the OLED.
 * - Functions are provided to send commands and data to the OLED display over I2C.
 * - The main function initializes the I2C and OLED, sets the cursor position, and writes temperature and humidity values to the display.
 *
 * @note
 * - The code assumes the use of an AVR microcontroller with TWI (I2C) hardware support.
 * - The OLED display is assumed to have an I2C address of 0x3C.
 * - The font array can be extended to include more characters as needed.
 *
 * @author
 * - Your Name
 * - Your Contact Information
 * - Date
 */
// main.c
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

// I2C Definitions
#define F_CPU 3333333UL
#define SCL_CLOCK  100000L
#define OLED_ADDR  0x3C

// OLED Commands
#define OLED_CMD_DISPLAY_OFF 0xAE
#define OLED_CMD_DISPLAY_ON  0xAF
#define OLED_CMD_SET_DISPLAY_CLOCK_DIV 0xD5
#define OLED_CMD_SET_MULTIPLEX 0xA8
#define OLED_CMD_SET_DISPLAY_OFFSET 0xD3
#define OLED_CMD_SET_START_LINE 0x40
#define OLED_CMD_CHARGE_PUMP 0x8D
#define OLED_CMD_MEMORY_MODE 0x20
#define OLED_CMD_SEG_REMAP 0xA1
#define OLED_CMD_COM_SCAN_DEC 0xC8
#define OLED_CMD_SET_CONTRAST 0x81
#define OLED_CMD_PRECHARGE 0xD9
#define OLED_CMD_SETVCOMDETECT 0xDB
#define OLED_CMD_DISPLAY_ALL_ON_RESUME 0xA4
#define OLED_CMD_NORMAL_DISPLAY 0xA6

// Simple 5x7 font
const uint8_t font[][5] PROGMEM = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 32 (space)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // 33 !
    // Add more characters as needed
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, // 48 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 49 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 50 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 51 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 52 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 53 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 54 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 55 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 56 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 57 9
    // Add more characters as needed
};

void i2c_init(void) {
    TWI0.MBAUD = (F_CPU / (2 * SCL_CLOCK)) - 5;
    TWI0.MCTRLA = TWI_ENABLE_bm;
}

void i2c_start(uint8_t address) {
    TWI0.MADDR = address << 1;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
}

void i2c_write(uint8_t data) {
    TWI0.MDATA = data;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
}

void i2c_stop(void) {
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;
}

void oled_command(uint8_t cmd) {
    i2c_start(OLED_ADDR);
    i2c_write(0x00); // Co = 0, D/C# = 0
    i2c_write(cmd);
    i2c_stop();
}

void oled_init(void) {
    oled_command(OLED_CMD_DISPLAY_OFF);
    oled_command(OLED_CMD_SET_DISPLAY_CLOCK_DIV);
    oled_command(0x80);
    oled_command(OLED_CMD_SET_MULTIPLEX);
    oled_command(0x1F);
    oled_command(OLED_CMD_SET_DISPLAY_OFFSET);
    oled_command(0x00);
    oled_command(OLED_CMD_SET_START_LINE | 0x00);
    oled_command(OLED_CMD_CHARGE_PUMP);
    oled_command(0x14);
    oled_command(OLED_CMD_MEMORY_MODE);
    oled_command(0x00);
    oled_command(OLED_CMD_SEG_REMAP | 0x01);
    oled_command(OLED_CMD_COM_SCAN_DEC);
    oled_command(OLED_CMD_SET_CONTRAST);
    oled_command(0x8F);
    oled_command(OLED_CMD_PRECHARGE);
    oled_command(0xF1);
    oled_command(OLED_CMD_SETVCOMDETECT);
    oled_command(0x40);
    oled_command(OLED_CMD_DISPLAY_ALL_ON_RESUME);
    oled_command(OLED_CMD_NORMAL_DISPLAY);
    oled_command(OLED_CMD_DISPLAY_ON);
}
   
void oled_set_cursor(uint8_t col, uint8_t row) {
    oled_command(0xB0 + row);
    oled_command(0x00 + (col & 0x0F));
    oled_command(0x10 + ((col >> 4) & 0x0F));
}

void oled_write_char(char c) {
    i2c_start(OLED_ADDR);
    i2c_write(0x40); // Co = 0, D/C# = 1
    for (uint8_t i = 0; i < 5; i++) {
        i2c_write(pgm_read_byte(&font[c - 32][i]));
    }
    i2c_write(0x00); // Space between characters
    i2c_stop();
}

void oled_write_string(const char *str) {
    while (*str) {
        oled_write_char(*str++);
    }
}

int main(void) {
    i2c_init();
    oled_init();

    oled_set_cursor(0, 0);
    oled_write_string("Temp: 25C");

    oled_set_cursor(0, 1);
    oled_write_string("Humidity: 50%");

    while (1) {
        
    }
}