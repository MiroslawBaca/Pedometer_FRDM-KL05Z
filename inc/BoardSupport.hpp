/*
 * Copyright (c) 2025 Miroslaw Baca
 * AGH - Design Lab
 */

/**
 * @file BoardSupport.hpp
 * @brief Manages board-level peripherals including ADC, LED, I2C, and TSI functions.
 */

#ifndef BOARD_SUPPORT_HPP
#define BOARD_SUPPORT_HPP

extern "C" {
#include "MKL05Z4.h"
}

#include <cstdint>

/**
 * @brief Basic macro for time delay.
 */
#ifndef DELAY
  #define DELAY(x)  for(uint32_t i = 0; i < (x * 10000U); i++) { __asm("nop"); }
#endif

/**
 * @brief Initializes the GPIO pins for the on-board RGB LED.
 */
void LED_init();

/**
 * @brief Sets the color of the on-board RGB LED.
 * @param r If true, enables the red LED.
 * @param g If true, enables the green LED.
 * @param b If true, enables the blue LED.
 */
void setLedColor(bool r, bool g, bool b);

/**
 * @namespace I2C
 * @brief Contains I2C-related methods for communication with external peripherals.
 */
namespace I2C
{
    /**
     * @brief Initializes the I2C0 peripheral for standard mode (~100kHz).
     */
    void init();

    /**
     * @brief Writes data to a specific register of an I2C device.
     * @param address 7-bit I2C device address.
     * @param reg Register address on the I2C device.
     * @param data Byte to write to the register.
     * @return 0 if successful, non-zero otherwise.
     */
    uint8_t writeReg(uint8_t address, uint8_t reg, uint8_t data);

    /**
     * @brief Reads a single byte from a specific register of an I2C device.
     * @param address 7-bit I2C device address.
     * @param reg Register address on the I2C device.
     * @param data Pointer to a variable where the read byte will be stored.
     * @return 0 if successful, non-zero otherwise.
     */
    uint8_t readReg(uint8_t address, uint8_t reg, uint8_t* data);

    /**
     * @brief Reads multiple bytes from consecutive registers of an I2C device.
     * @param address 7-bit I2C device address.
     * @param reg Starting register address on the I2C device.
     * @param size Number of bytes to read.
     * @param data Pointer to a buffer where the read bytes will be stored.
     * @return 0 if successful, non-zero otherwise.
     */
    uint8_t readRegBlock(uint8_t address, uint8_t reg, uint8_t size, uint8_t* data);
}


#endif // BOARD_SUPPORT_HPP
