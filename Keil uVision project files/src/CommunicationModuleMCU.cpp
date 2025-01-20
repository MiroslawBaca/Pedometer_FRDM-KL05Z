/*
 * Copyright (c) 2024 Miroslaw Baca
 * AGH - Object-Oriented Programming Language
 */

/**
 * @file CommunicationModuleMCU.cpp
 * @brief Implementation of CommunicationModuleMCU for the KL05Z MCU.
 */

#include "../inc/CommunicationModuleMCU.hpp"
#include "../inc/Uart.hpp"
#include <cstdio>
#include "../inc/BoardSupport.hpp"

namespace mb { // Start of namespace mb

CommunicationModuleMCU::CommunicationModuleMCU()
{
}


void CommunicationModuleMCU::init()
{
    std::memset(buffer, 0, sizeof(buffer));
    dataReady = false;
}

void CommunicationModuleMCU::print(const char* text)
{
    Uart::print(text);
}

void CommunicationModuleMCU::println(const char* text)
{
    Uart::println(text);
}

char* CommunicationModuleMCU::receiveData()
{
    while (!dataReady)
    {
        // Wait until data is ready
    }
    dataReady = false;
    return buffer;
}

void CommunicationModuleMCU::onCharReceived(char c)
{
    static size_t idx = 0;

    if (c == '\n')
    {
        buffer[idx] = '\0';
        idx = 0;
        dataReady = true;
    }
    else if (c == '\r')
    {
        // Ignore carriage return
    }
    else
    {
        if (idx < (BUFFER_SIZE - 1))
        {
            buffer[idx++] = c;
        }
    }
}

} // End of namespace mb