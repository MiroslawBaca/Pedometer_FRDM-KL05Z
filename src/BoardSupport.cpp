/*
 * Copyright (c) 2025 Miroslaw Baca
 * AGH - Design Lab
 */

/**
 * @file BoardSupport.cpp
 * @brief Implementation of board-level functions for LED, I2C.
 */

#include "../inc/BoardSupport.hpp"

/* =========================================
 * LED Functions
 * =========================================
 */

void LED_init()
{
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;

    PORTB->PCR[8]  = PORT_PCR_MUX(1);
    PORTB->PCR[9]  = PORT_PCR_MUX(1);
    PORTB->PCR[10] = PORT_PCR_MUX(1);

    PTB->PDDR |= (1u << 8) | (1u << 9) | (1u << 10);
    PTB->PSOR = (1u << 8) | (1u << 9) | (1u << 10);
}

void setLedColor(bool r, bool g, bool b)
{
    uint32_t maskClear = 0;
    uint32_t maskSet   = 0;

    if (r) { maskClear |= (1u << 8); }  else { maskSet |= (1u << 8); }
    if (g) { maskClear |= (1u << 9); }  else { maskSet |= (1u << 9); }
    if (b) { maskClear |= (1u << 10); } else { maskSet |= (1u << 10); }

    PTB->PCOR = maskClear;
    PTB->PSOR = maskSet;
}

/* =========================================
 * I2C
 * =========================================
 */

namespace I2C
{
    static inline uint8_t  error   = 0;
    static inline uint16_t timeout = 0;

    static void i2c_enable()   { I2C0->C1 |=  I2C_C1_IICEN_MASK; }
    static void i2c_disable()  { I2C0->C1 &= ~I2C_C1_IICEN_MASK; }
    static void i2c_m_start()  { I2C0->C1 |=  I2C_C1_MST_MASK; }
    static void i2c_m_stop()   { I2C0->C1 &= ~I2C_C1_MST_MASK; }
    static void i2c_m_rstart() { I2C0->C1 |=  I2C_C1_RSTA_MASK; }
    static void i2c_tran()     { I2C0->C1 |=  I2C_C1_TX_MASK; }
    static void i2c_rec()      { I2C0->C1 &= ~I2C_C1_TX_MASK; }
    static void i2c_nack()     { I2C0->C1 |=  I2C_C1_TXAK_MASK; }
    static void i2c_ack()      { I2C0->C1 &= ~I2C_C1_TXAK_MASK; }
    static void i2c_send(uint8_t d) { I2C0->D = d; }
    static uint8_t i2c_recv()       { return I2C0->D; }

    static void i2c_wait()
    {
        timeout = 0;
        while (!(I2C0->S & I2C_S_IICIF_MASK) && (timeout < 10000))
        {
            ++timeout;
        }
        I2C0->S |= I2C_S_IICIF_MASK;
    }

    void init()
    {
        SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK;
        SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;

        PORTB->PCR[3] = PORT_PCR_MUX(2);
        PORTB->PCR[4] = PORT_PCR_MUX(2);

        I2C0->C1 &= ~I2C_C1_IICEN_MASK;
        I2C0->F = 0x03;
    }

    uint8_t writeReg(uint8_t address, uint8_t reg, uint8_t data)
    {
        error = 0;
        i2c_enable();
        i2c_tran();
        i2c_m_start();
        i2c_send(static_cast<uint8_t>(address << 1));
        i2c_wait();
        i2c_send(reg);
        i2c_wait();
        i2c_send(data);
        i2c_wait();
        i2c_m_stop();
        i2c_disable();
        return error;
    }

    uint8_t readReg(uint8_t address, uint8_t reg, uint8_t* data)
    {
        error = 0;
        i2c_enable();
        i2c_tran();
        i2c_m_start();
        i2c_send(static_cast<uint8_t>(address << 1));
        i2c_wait();
        i2c_send(reg);
        i2c_wait();
        i2c_m_rstart();
        i2c_send(static_cast<uint8_t>((address << 1) | 1u));
        i2c_wait();
        i2c_rec();
        i2c_nack();
        (void)i2c_recv();
        i2c_wait();
        i2c_m_stop();
        *data = i2c_recv();
        i2c_disable();
        return error;
    }

    uint8_t readRegBlock(uint8_t address, uint8_t reg, uint8_t size, uint8_t* data)
    {
        error = 0;
        uint8_t dummy;
        uint8_t cnt = 0;

        i2c_enable();
        i2c_tran();
        i2c_m_start();
        i2c_send(static_cast<uint8_t>(address << 1));
        i2c_wait();
        i2c_send(reg);
        i2c_wait();
        i2c_m_rstart();
        i2c_send(static_cast<uint8_t>((address << 1) | 0x01));
        i2c_wait();
        i2c_rec();

        while (cnt < (size - 1))
        {
            i2c_ack();
            dummy = i2c_recv();
            (void)dummy;
            i2c_wait();
            data[cnt++] = i2c_recv();
        }

        i2c_nack();
        dummy = i2c_recv();
        (void)dummy;
        i2c_wait();
        i2c_m_stop();
        data[cnt] = i2c_recv();
        i2c_disable();
        return error;
    }
} // End of namespace I2C