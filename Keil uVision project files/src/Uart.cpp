/*
 * Copyright (c) 2024 Miroslaw Baca
 * AGH - Object-Oriented Programming Language
 */

/**
 * @file Uart.cpp
 * @brief Implementation of Uart class for the KL05Z MCU.
 */

#include "../inc/Uart.hpp"
#include "../inc/CommunicationModuleMCU.hpp"

mb::CommunicationModuleMCU* mb::Uart::g_commObject = nullptr;

extern "C" void UART0_IRQHandler(void)
{
	mb::Uart::handleIRQ();
}

namespace mb {

Uart::Uart() : Uart(9600, nullptr)
{
}

Uart::Uart(uint32_t baud, CommunicationModuleMCU* obj) : baudRate(baud)
{
		// Store the pointer to the communication handler object
    g_commObject = obj;

    // Update the system clock frequency to ensure correct timing for peripherals (after changes I made to #define CLOCK_SETUP 1)
    SystemCoreClockUpdate();

    // Enable clock for UART0 module
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;

    // Enable clock for Port B (used for UART0 TX and RX pins)
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;

    // Select MCGFLLCLK as the clock source for UART0
    SIM->SOPT2 |= (1u << 26);

    // Configure Port B pins for UART0 functionality:
    // PTB1 is used for UART0_TX (alternate function 2)
    PORTB->PCR[1] = PORT_PCR_MUX(2);

    // PTB2 is used for UART0_RX (alternate function 2)
    PORTB->PCR[2] = PORT_PCR_MUX(2);

    // Disable the UART transmitter and receiver before making changes
    UART0->C2 &= ~(UART0_C2_TE_MASK | UART0_C2_RE_MASK);

    // Calculate the baud rate divisor:
    // UART baud rate = Clock frequency / (16 * divisor)
    uint32_t divisor = 48000000u / (16u * baudRate);

    // Set the upper 5 bits of the divisor in the BDH register
    UART0->BDH = (uint8_t)((divisor >> 8) & 0x1F);

    // Set the lower 8 bits of the divisor in the BDL register
    UART0->BDL = (uint8_t)(divisor & 0xFF);

    // Set fine-tuning value for the baud rate (optional adjustment)
    UART0->C4 = 0x0F;

    // Enable UART receive interrupt (RIE)
    UART0->C2 |= UART0_C2_RIE_MASK;

    // Enable the UART transmitter (TE) and receiver (RE)
    UART0->C2 |= (UART0_C2_TE_MASK | UART0_C2_RE_MASK);

    // Enable UART0 interrupt in the Nested Vector Interrupt Controller (NVIC)
    NVIC_EnableIRQ(UART0_IRQn);
}

Uart::Uart(const Uart& other) : Uart(other.baudRate, other.g_commObject)
{
}

Uart::~Uart()
{
    NVIC_DisableIRQ(UART0_IRQn);
}

void Uart::print(const char* text)
{
    while (*text)
    {
        sendChar(*text++);
    }
}

void Uart::println(const char* text)
{
    print(text);
    print("\n\r");
}

void Uart::sendChar(char c)
{
    while (!(UART0->S1 & UART0_S1_TDRE_MASK))
    {
        // Wait until the transmitter is ready
    }
    UART0->D = static_cast<uint8_t>(c);
}

void Uart::handleIRQ()
{
    while (UART0->S1 & UART0_S1_RDRF_MASK)
    {
        char c = static_cast<char>(UART0->D);
        if (g_commObject)
        {
            g_commObject->onCharReceived(c);
        }
    }
}

} // End of namespace mb