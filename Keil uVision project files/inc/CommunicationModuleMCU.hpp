/*
 * Copyright (c) 2024 Miroslaw Baca
 * AGH - Object-Oriented Programming Language
 */

/**
 * @file CommunicationModuleMCU.hpp
 * @brief Concrete communication module that processes commands on the KL05Z MCU.
 */

#ifndef COMMUNICATION_MODULE_MCU_HPP
#define COMMUNICATION_MODULE_MCU_HPP

#include <cstdint>
#include <cstring>


/**
 * @class CommunicationModuleMCU
 * @brief Implements a communication interface for the microcontroller,
 *        handling command parsing and buffering of incoming data.
 */
 
namespace mb {
	 
class CommunicationModuleMCU
{
private:
    static constexpr size_t BUFFER_SIZE = 64; /**< Size of the local buffer. */
    char buffer[BUFFER_SIZE];                 /**< Local buffer for received data. */
    volatile bool dataReady = false;          /**< Flag indicating new data availability. */

public:
    /**
     * @brief Default constructor.
     */
    CommunicationModuleMCU();

    /**
     * @brief Initializes the higher-level communication (e.g., resets local buffer).
     */
    void init();

    /**
     * @brief Sends a string via UART without a trailing newline.
     * @param text Pointer to the string buffer.
     */
    void print(const char* text);

    /**
     * @brief Sends a string via UART and appends \n\r.
     * @param text Pointer to the string buffer.
     */
    void println(const char* text);

    /**
     * @brief Blocks until data is ready and then returns a pointer to the received buffer.
     * @return Pointer to the local buffer containing the received string.
     */
    char* receiveData();

    /**
     * @brief Called from the UART interrupt to store a received character.
     * @param c Received character.
     */
    void onCharReceived(char c);
		
};

} // End of namespace mb
#endif // COMMUNICATION_MODULE_MCU_HPP
