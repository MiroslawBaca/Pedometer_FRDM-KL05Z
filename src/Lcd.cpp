/*
 * Copyright (c) 2025 Miroslaw Baca
 * AGH - Design Lab
 */
 
 /**
 * @file lcd.cpp
 * @brief Implementation of the LCD 16x2 driver using the PCF8574 I2C expander.
 *
 * This file contains the logic to initialize and control a standard HD44780-based
 * 16x2 character LCD display via the PCF8574 I2C port expander. It re-implements
 * minimal I2C transactions (start, stop, send byte) locally because the existing
 * BoardSupport I2C library only provides register-based functions.
 */

#include "../inc/Lcd.hpp"  // NEW
#include "../inc/BoardSupport.hpp"

/* Commands for HD44780 */
constexpr uint8_t LCD_CLEAR_DISPLAY = 0x01;
constexpr uint8_t LCD_SET_DDRAMADDR = 0x80;
constexpr uint8_t LCD_FULLLINE      = 0x40;  // offset for row 2

/* PCF8574 default addresses */
constexpr uint8_t PCF8574_ADDRESS  = 0x27;  // typical address
constexpr uint8_t PCF8574A_ADDRESS = 0x3F;  // alternate address

/* PCF8574 bit masks for LCD control */
constexpr uint8_t PCF8574_BL = 0x08;  // Backlight
constexpr uint8_t PCF8574_EN = 0x04;  // Enable bit
constexpr uint8_t PCF8574_RS = 0x01;  // Register select bit

/* Internal static variables */
bool    g_lcdBacklight  = true;        /**< Tracks backlight state */
uint8_t g_pcfAddress    = PCF8574_ADDRESS;

/* =====================================================
 * Internal "raw" I2C helper functions - local replication
 * =====================================================
 */
static inline void i2c_enable()
{
    I2C0->C1 |= I2C_C1_IICEN_MASK;
}
static inline void i2c_disable()
{
    I2C0->C1 &= ~I2C_C1_IICEN_MASK;
}
static inline void i2c_start()
{
    I2C0->C1 |= I2C_C1_MST_MASK;
}
static inline void i2c_stop()
{
    I2C0->C1 &= ~I2C_C1_MST_MASK;
}
static inline void i2c_tran()
{
    I2C0->C1 |= I2C_C1_TX_MASK;
}
static inline void i2c_send(uint8_t data)
{
    I2C0->D = data;
}
static inline void i2c_wait(uint8_t& error)
{
    uint16_t timeout = 0;
    while ((!(I2C0->S & I2C_S_IICIF_MASK)) && (timeout < 10000))
    {
        ++timeout;
    }
    I2C0->S |= I2C_S_IICIF_MASK; // clear the flag

    if (timeout >= 10000)
    {
        // optional: set an error code if needed
        error = 1;
    }

    // Check if no ACK
    if (I2C0->S & I2C_S_RXAK_MASK)
    {
        error = 2;
    }
}

/**
 * @brief Writes a single byte to the given I2C address (no register).
 * @param address 7-bit address of the slave.
 * @param data Data byte to send.
 * @return 0 if success, non-zero if any error (timeout or NACK).
 */
uint8_t i2c_writeByte(uint8_t address, uint8_t data)
{
    uint8_t error = 0;

    i2c_enable();
    i2c_tran();
    i2c_start();

    i2c_send(static_cast<uint8_t>(address << 1));
    i2c_wait(error);
    if (error != 0)
    {
        i2c_stop();
        i2c_disable();
        return error;
    }

    i2c_send(data);
    i2c_wait(error);
    i2c_stop();
    i2c_disable();

    return error;
}

/* =====================================================
 * Private local functions replicating lcd1602.c logic
 * =====================================================
 */

/**
 * @brief Writes a single byte to the PCF8574 expander (with backlight info).
 * @param data Data to send.
 */
void PCF8574_Write(uint8_t data)
{
    // If backlight is on, OR with PCF8574_BL bit
    uint8_t toSend = data | (g_lcdBacklight ? PCF8574_BL : 0x00);
    i2c_writeByte(g_pcfAddress, toSend);
}

/**
 * @brief Send 4 bits to LCD (lower nibble of @p data is ignored).
 * @param data Upper nibble to send (0-15).
 * @param rs Register select (0 = command, 1 = data).
 */
void LCD_Write4(uint8_t data, bool rs)
{
    uint8_t highNibble = (data << 4) & 0xF0;  // mask upper nibble
    uint8_t control = (rs ? PCF8574_RS : 0x00);

    // EN = 1
    PCF8574_Write(highNibble | control | PCF8574_EN);
    // EN = 0
    PCF8574_Write(highNibble | control);
    DELAY(2);
}

/**
 * @brief Send a full 8-bit command or data to LCD in two steps of 4 bits.
 * @param data Byte to send.
 * @param rs Register select (false = command, true = data).
 */
void LCD_Write8(uint8_t data, bool rs)
{
    // High nibble first
    LCD_Write4((data >> 4) & 0x0F, rs);
    // Then low nibble
    LCD_Write4(data & 0x0F, rs);
}

/**
 * @brief Tries to detect which PCF8574 address is responding (0x27 or 0x3F).
 */
void checkPCFaddress()
{
    // Check if 0x27 answers
    uint8_t err = i2c_writeByte(PCF8574_ADDRESS, 0x00);
    if (err == 0)
    {
        g_pcfAddress = PCF8574_ADDRESS;
    }

    // Check if 0x3F answers
    err = i2c_writeByte(PCF8574A_ADDRESS, 0x00);
    if (err == 0)
    {
        g_pcfAddress = PCF8574A_ADDRESS;
    }
}


/* =====================================================
 * Public API (namespace lcd)
 * =====================================================
 */
namespace lcd
{

void init()
{
    // Initialize the MCU I2C from BoardSupport
    I2C::init();

    // Check which PCF address is valid
    checkPCFaddress();

    // Wait >15ms after power up (HD44780 datasheet)
    DELAY(42);

    // Initialize in 4-bit mode
    // Sequence recommended in many HD44780 references
    LCD_Write8(0x33, false); // Command: 0x33
    LCD_Write8(0x32, false); // Command: 0x32 (set to 4-bit mode)
    LCD_Write8(0x2C, false); // Function set: 4-bit, 2 lines, 5x8 font
    LCD_Write8(0x08, false); // Display off, cursor off, blink off
    LCD_Write8(0x01, false); // Clear display
    LCD_Write8(0x0C, false); // Display on, cursor off, blink off
}

void clearAll()
{
    LCD_Write8(LCD_CLEAR_DISPLAY, false);
}

void print(const char* str)
{
    if (!str)
        return;

    while (*str)
    {
        LCD_Write8(static_cast<uint8_t>(*str), true); // rs=1 => data
        ++str;
    }
}

void setCursor(uint8_t col, uint8_t row)
{
    if (row > 1)
        row = 1; // for 2-line display, max row = 1

    uint8_t address = static_cast<uint8_t>(LCD_SET_DDRAMADDR + col + (LCD_FULLLINE * row));
    LCD_Write8(address, false);
}

void backlight(bool state)
{
    g_lcdBacklight = state;
    // Force a write to refresh backlight state
    PCF8574_Write(0x00);
}

void blinkOn()
{
    // 0x0D => display on, cursor off, blink on
    LCD_Write8(0x0D, false);
}

void blinkOff()
{
    // 0x0C => display on, cursor off, blink off
    LCD_Write8(0x0C, false);
}

} // end namespace lcd
