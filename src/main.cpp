/*
 * Copyright (c) 2025 Miroslaw Baca
 * AGH - Design Lab
 */

/**
 * @file main.cpp
 * @brief Main entry point of the FRDM-KL05Z application. 
 *        Using an interrupt on Port A pin to detect button press.
 */

#include <cstdio>
#include <cstdint>

extern "C" {
#include "MKL05Z4.h"
}

#include "../inc/BoardSupport.hpp"
#include "../inc/Uart.hpp"
#include "../inc/lcd.hpp"

/* =============== IMPORTANT NOTES ===============
 * In this project, I made the following changes in system_MKL05Z4.c file:
 *
 * Updated Stack_Size:
 * EQU     0x0400
 * (Original value: 0x00000100)
 *
 * Modified clock setup:
 * #define CLOCK_SETUP 1
 * (Original value: 0)
 * ===============================================
 */

/**
 * @brief Basic macro for time delay.
 */
#ifndef DELAY
  #define DELAY(x)  for(uint32_t i = 0; i < (x * 10000U); i++) { __asm("nop"); }
#endif

#define BUTTON_PIN_POS 11

/**
 * @brief PORTA Interrupt Service Routine.
 */

uint32_t WalkStep = 0;
uint32_t RunStep = 0;
	
extern "C" void PORTA_IRQHandler(void)
{
    // Check if the interrupt is indeed from our pin:
    if (PORTA->ISFR & (1 << BUTTON_PIN_POS))
    {
        // Clear the interrupt status flag by writing 1
        PORTA->ISFR |= (1 << BUTTON_PIN_POS);

					// Inform about reset
					lcd::setCursor(0, 1);
					lcd::print("Reseting steps..");
					DELAY(1000);

					// Reset values
					lcd::clearAll();
					WalkStep = 0;
					RunStep = 0;
					lcd::setCursor(0, 0);
					lcd::print("Start moving");
					lcd::setCursor(0, 1);
					lcd::print("to count steps");
    }
}




int main()
{
    // UART initialization for debug/print
    Uart start(9600);

    // Board peripherals initialization
    I2C::init();
    LED_init();

    // Green LED on)
    setLedColor(false, true, false);

    // === BUTTON CONFIGURATION ON PORTA ===
    // Enable clock on PORTA
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    // Configure pin 11 as GPIO with pull-up resistor
    PORTA->PCR[BUTTON_PIN_POS] = PORT_PCR_MUX(1)   // GPIO
                               | PORT_PCR_PE_MASK  // Pull Enable
                               | PORT_PCR_PS_MASK; // Pull Select -> up
    // Set pin as input
    PTA->PDDR &= ~(1 << BUTTON_PIN_POS);

    // Configure interrupt on falling edge
    // IRQC(0b1010) => interrupt on falling edge
    PORTA->PCR[BUTTON_PIN_POS] &= ~PORT_PCR_IRQC_MASK;
    PORTA->PCR[BUTTON_PIN_POS] |= PORT_PCR_IRQC(0xA);

    // Enable interrupt in NVIC
    NVIC_ClearPendingIRQ(PORTA_IRQn);
    NVIC_EnableIRQ(PORTA_IRQn);

    // === LCD INITIALIZATION ===
    lcd::init();
    lcd::clearAll();

    // Display initial messages
    lcd::setCursor(0, 0);
    lcd::print("Start moving");
    lcd::setCursor(0, 1);
    lcd::print("to count steps");


		// === UART RX Interrupt Configuration ===
    // RIE – Receive Interrupt Enable
    UART0->C2 |= UART0_C2_RIE_MASK;
    NVIC_ClearPendingIRQ(UART0_IRQn);
    NVIC_EnableIRQ(UART0_IRQn);
		

    while (true)
    {
			// Accelerometer config & reading
			I2C::writeReg(0x1D, 0x2A, 1);
			I2C::writeReg(0x1D, 0x0E, 0x00);

			static char tempBuffer[36];
			static uint8_t arrayXYZ[6];
			I2C::readRegBlock(0x1D, 0x01, 6, arrayXYZ);
			
			double x_=((double)((int16_t)((arrayXYZ[0]<<8)|arrayXYZ[1])>>2)/4096);
			double y_=((double)((int16_t)((arrayXYZ[2]<<8)|arrayXYZ[3])>>2)/4096);
			double z_=((double)((int16_t)((arrayXYZ[4]<<8)|arrayXYZ[5])>>2)/4096);
			sprintf(tempBuffer,"%1.4f  %1.4f  %1.4f", x_, y_, z_); // default 4096 counts/g sensitivity
			start.println(tempBuffer);


			// 10 Hz
			DELAY(98); // Assuming a 2ms delay due to code processing
    }
			
    return 0;
}
