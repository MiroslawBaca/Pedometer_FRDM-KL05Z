/*
 * Copyright (c) 2024 Miroslaw Baca
 * AGH - Object-Oriented Programming Language
 */

/**
 * @file Uart.hpp
 * @brief Provides UART0 functionality for data transmission on the KL05Z MCU.
 */

#ifndef UART_HPP
#define UART_HPP

#include <cstdint>

extern "C" {
#include "MKL05Z4.h"
}

extern "C" void UART0_IRQHandler(void);


namespace mb { // Start of namespace mb
	
class CommunicationModuleMCU;       // Forward declaration

/**
 * @class Uart
 * @brief Class for initializing and handling UART0 transmissions and interrupts.
 */
class Uart
{
private:
    uint32_t baudRate;                         /**< Baud rate for UART communication. */
    static CommunicationModuleMCU* g_commObject;/**< Pointer to the communication handler. */

public:
    /**
     * @brief Default constructor, initializes UART with 9600 baud.
     */
    Uart();

    /**
     * @brief Parameterized constructor, sets baud rate and a communication handler.
     * @param baud Desired baud rate.
     * @param obj Pointer to the communication handler.
     */
    Uart(uint32_t baud, CommunicationModuleMCU* obj = nullptr);

    /**
     * @brief Copy constructor.
     * @param other Reference to another Uart object.
     */
    Uart(const Uart& other);

    /**
     * @brief Destructor, disables the UART interrupt.
     */
    ~Uart();

    /**
     * @brief Sends a null-terminated string via UART (without a new line).
     * @param text Pointer to the string buffer.
     */
    static void print(const char* text);

    /**
     * @brief Sends a null-terminated string via UART, then appends \n\r.
     * @param text Pointer to the string buffer.
     */
    static void println(const char* text);

private:
    /**
     * @brief Sends a single character in a blocking manner.
     * @param c Character to send.
     */
    static void sendChar(char c);

    /**
     * @brief Handles the UART interrupt, reads the received character,
     *        and forwards it to the communication handler if present.
     */
    static void handleIRQ();

    friend void ::UART0_IRQHandler(); // Allows the global IRQ handler to invoke private handleIRQ().
};
} // End of namespace mb
#endif // UART_HPP
