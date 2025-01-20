/*
 * Copyright (c) 2024 Miroslaw Baca
 * AGH - Object-Oriented Programming Language
 */

/**
 * @file BoardSupport.cpp
 * @brief Implementation of board-level functions for ADC, LED, I2C, and TSI.
 */

#include "../inc/BoardSupport.hpp"

/* =========================================
 * ADC Functions
 * =========================================
 */

uint8_t ADC_Init()
{
    uint16_t kalib_temp;
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    PORTA->PCR[12] &= ~PORT_PCR_MUX_MASK;

    ADC0->CFG1 = ADC_CFG1_ADICLK(1)
               | ADC_CFG1_ADIV(2)
               | ADC_CFG1_ADLSMP_MASK
               | ADC_CFG1_MODE(1);
    ADC0->CFG2 = ADC_CFG2_ADHSC_MASK;
    ADC0->SC3  = ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3);

    ADC0->SC3 |= ADC_SC3_CAL_MASK;
    while (ADC0->SC3 & ADC_SC3_CAL_MASK)
    {
        // Wait for calibration to finish
    }
    if (ADC0->SC3 & ADC_SC3_CALF_MASK)
    {
        return 1; 
    }

    kalib_temp = ADC0->CLP0
               + ADC0->CLP1
               + ADC0->CLP2
               + ADC0->CLP3
               + ADC0->CLP4
               + ADC0->CLPS;
    kalib_temp = static_cast<uint16_t>((kalib_temp / 2) | 0x8000u);
    ADC0->PG = kalib_temp;

    ADC0->SC1[0] = ADC_SC1_ADCH(31);
    return 0;
}

uint16_t ADC_ReadChannel(uint8_t channel)
{
    ADC0->SC1[0] = ADC_SC1_ADCH(channel);
    while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
    {
        // Wait for conversion to complete
    }
    return ADC0->R[0];
}

float readTemperature()
{
    uint16_t adcValue = ADC_ReadChannel(26);
    float voltage = (adcValue * 2.91f) / 4095.0f;
    float result = 25.0f - (voltage - 0.716f) / 0.00162f;
    return result;
}

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
 * I2C Namespace
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

/* =========================================
 * TSI (Touch Sensing) Functions
 * =========================================
 */

#define TOTAL_ELECTRODE 2
static constexpr uint8_t ELECTRODE0  = 9;
static constexpr uint8_t ELECTRODE1  = 8;
static constexpr uint16_t THRESHOLD0 = 100;
static constexpr uint16_t THRESHOLD1 = 100;

static uint16_t gu16TSICount[TOTAL_ELECTRODE];
static uint16_t gu16Baseline[TOTAL_ELECTRODE];
static uint16_t gu16Delta[TOTAL_ELECTRODE];
static uint8_t ongoing_elec = 0;
static uint8_t end_flag = 1;

static uint8_t elec_array[TOTAL_ELECTRODE] = { ELECTRODE0, ELECTRODE1 };

static void change_electrode()
{
    gu16TSICount[ongoing_elec] = static_cast<uint16_t>(TSI0->DATA & 0xFFFFu);
    int16_t delta = static_cast<int16_t>(gu16TSICount[ongoing_elec] - gu16Baseline[ongoing_elec]);
    gu16Delta[ongoing_elec] = (delta < 0) ? 0 : static_cast<uint16_t>(delta);

    if (ongoing_elec < (TOTAL_ELECTRODE - 1))
    {
        ongoing_elec++;
    }
    else
    {
        ongoing_elec = 0;
    }
    TSI0->DATA = (static_cast<uint32_t>(elec_array[ongoing_elec]) << 28) | (1u << 22);
}

void self_calibration()
{
    TSI0->GENCS |= TSI_GENCS_EOSF_MASK;
    TSI0->GENCS &= ~TSI_GENCS_TSIEN_MASK;

    uint8_t stm_backup = ((TSI0->GENCS & (1u << 1)) != 0) ? 1 : 0;
    TSI0->GENCS &= ~((1u << 1) | TSI_GENCS_TSIIEN_MASK);
    TSI0->GENCS |= TSI_GENCS_TSIEN_MASK;

    for (uint8_t cnt = 0; cnt < TOTAL_ELECTRODE; cnt++)
    {
        TSI0->DATA = (static_cast<uint32_t>(elec_array[cnt]) << 28) | (1u << 22);
        while (!(TSI0->GENCS & TSI_GENCS_EOSF_MASK))
        {
            // Wait for TSI measurement
        }
        TSI0->GENCS |= TSI_GENCS_EOSF_MASK;
        gu16Baseline[cnt] = static_cast<uint16_t>(TSI0->DATA & 0xFFFFu);
    }

    TSI0->GENCS &= ~TSI_GENCS_TSIEN_MASK;
    TSI0->GENCS |= TSI_GENCS_TSIIEN_MASK;
    if (stm_backup) { TSI0->GENCS |= (1u << 1); }
    else            { TSI0->GENCS &= ~(1u << 1); }

    TSI0->GENCS |= TSI_GENCS_TSIEN_MASK;
    ongoing_elec = 0;
    TSI0->DATA = (static_cast<uint32_t>(elec_array[0]) << 28) | (1u << 22);
}

void TSI_Init()
{
    SIM->SCGC5 |= SIM_SCGC5_TSI_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK;

    PORTA->PCR[13] = PORT_PCR_MUX(0);
    PORTB->PCR[12] = PORT_PCR_MUX(0);

    TSI0->GENCS |= ((1u << 28) | (0u << 3) | (4u << 24) | (0u << 19)
                  | (7u << 20) | (4u << 0) | (11u << 8)
                  | (1u << 6)  | (1u << 5) | (1u << 7));

    NVIC_ClearPendingIRQ(TSI0_IRQn);
    NVIC_EnableIRQ(TSI0_IRQn);

    self_calibration();
}

uint8_t TSI_ReadSlider()
{
    if (!end_flag)
    {
        return 0;
    }
    end_flag = 0;

    if ((gu16Delta[0] > THRESHOLD0) || (gu16Delta[1] > THRESHOLD1))
    {
        int a = (gu16Delta[0] * 100) / (gu16Delta[0] + gu16Delta[1]);
        int b = (gu16Delta[1] * 100) / (gu16Delta[0] + gu16Delta[1]);
        return static_cast<uint8_t>((100 - a + b) / 2);
    }
    else
    {
        return 0;
    }
}

extern "C" void TSI0_IRQHandler(void)
{
    end_flag = 1;
    TSI0->GENCS |= TSI_GENCS_EOSF_MASK;
    change_electrode();
}
