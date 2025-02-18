/*
 * Copyright (c) 2025 Miroslaw Baca
 * AGH - Design Lab
 */
 
 /**
 * @file lcd.hpp
 * @brief A driver for controlling a 16x2 LCD display (HD44780-based) through the PCF8574 I2C expander.
 *
 * This header provides a C++ interface to initialize and manipulate the LCD1602 display.
 * It replicates functionality similar to the classic C driver but integrates into the
 * existing I2C code from BoardSupport.
 *
 * Usage:
 * @code
 *   lcd::init();
 *   lcd::clearAll();
 *   lcd::setCursor(0, 0);
 *   lcd::print("Hello!");
 * @endcode
 *
 * @copyright Copyright (c) 2025 YourName
 */

#ifndef LCD_HPP
#define LCD_HPP

#include <cstdint>

/**
 * @namespace lcd
 * @brief Namespace containing functions to control a 16x2 LCD display over I2C with a PCF8574 expander.
 */
namespace lcd
{
    /**
     * @brief Initializes the LCD in 4-bit mode and configures basic parameters like cursor and blink.
     */
    void init();

    /**
     * @brief Clears the entire display (both lines).
     */
    void clearAll();

    /**
     * @brief Prints a null-terminated string on the display at the current cursor position.
     * @param str Pointer to a null-terminated string (const char*).
     */
    void print(const char* str);

    /**
     * @brief Moves the cursor to the specified column and row.
     * @param col Column index (0-based).
     * @param row Row index (0-based, max 1 for 16x2 display).
     */
    void setCursor(uint8_t col, uint8_t row);

    /**
     * @brief Enables or disables the LCD backlight (if the PCF8574 board supports it).
     * @param state True to enable the backlight, false to disable.
     */
    void backlight(bool state);

    /**
     * @brief Enables the blinking of the cursor.
     */
    void blinkOn();

    /**
     * @brief Disables the blinking of the cursor.
     */
    void blinkOff();
}

#endif // LCD_HPP
