/*
 * Copyright (c) 2024 Miroslaw Baca
 * AGH - Object-Oriented Programming Language
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
 * @brief Initializes the ADC peripheral.
 * @return 0 if successful, otherwise non-zero if calibration failed.
 */
uint8_t ADC_Init();

/**
 * @brief Reads the specified ADC channel.
 * @param channel Channel number to read.
 * @return The ADC conversion result (12-bit or 16-bit, depending on configuration).
 */
uint16_t ADC_ReadChannel(uint8_t channel);

/**
 * @brief Reads and calculates the internal temperature from ADC data.
 * @return Temperature in degrees Celsius.
 */
float readTemperature();

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

/**
 * @brief Initializes the TSI (Touch Sensing Input) peripheral.
 */
void TSI_Init();

/**
 * @brief Reads the value from a simple touch slider.
 * @return Slider value in the range 0-100 (approx.), or 0 if not touched.
 */
uint8_t TSI_ReadSlider();

/**
 * @brief Performs self-calibration for TSI measurements.
 */
void self_calibration();

#endif // BOARD_SUPPORT_HPP
