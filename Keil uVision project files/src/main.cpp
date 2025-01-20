/*
 * Copyright (c) 2024 Miroslaw Baca
 * AGH - Object-Oriented Programming Language
 */

/**
 * @file main.cpp
 * @brief Main entry point of the FRDM-KL05Z application.
 */

#include <cstdio>
#include <cstdint>

extern "C" {
#include "MKL05Z4.h"
}

#include "../inc/BoardSupport.hpp"
#include "../inc/Uart.hpp"
#include "../inc/CommunicationModuleMCU.hpp"

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

int main()
{


		mb::CommunicationModuleMCU comm_obj;
		mb::Uart start(9600, &comm_obj);

    I2C::init();
    LED_init();
    TSI_Init();
    ADC_Init();

    comm_obj.init();
    setLedColor(true, false, false);
    comm_obj.println("Waiting for commands...");

    while (true)
    {
        // Wait for incoming data and handle the received command.
        //char* msg = comm_obj.receiveData();
        //comm_obj.handleCommand(msg);
    }

    return 0;
}
