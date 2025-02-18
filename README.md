# **Step Counter/Pedometer using FRDM-KL05Z & MATLAB**

This repository contains a real-time **step counting system** based on the **FRDM-KL05Z** microcontroller and **MATLAB** for precise data processing/filtering and visualization.


## **Project Overview**

### **Microcontroller Firmware (C++ Code)**
- **Modular Structure:**  
  Developed in C++ using Keil uVision with a clear separation into modules for improved readability, maintainability, and scalability. The project consists of:
  - **main.cpp:** Configures system peripherals, sets up UART communication on serial port, initializes the LCD via I²C, and handles button interrupts to reset step counters.
  - **Uart.cpp/Uart.hpp:** Implements the UART communication interface, including initialization, data transmission, and interrupt-driven reception. The UART interrupt handler processes incoming commands ("WALK++" or "RUN++") and updates step counters accordingly.
  - **BoardSupport.cpp/BoardSupport.hpp:** Provides low-level functions for initializing and controlling peripherals such as I²C, LED.
  - **Lcd.cpp/Lcd.hpp:** Implements the LCD driver for a 16×2 HD44780 display using a PCF8574 I²C expander, handling initialization, cursor positioning, and display functions.


### **MATLAB Data Processing & Visualization**
  - **Real-Time Data Acquisition:**  
    The MATLAB script establishes a serial connection (in this exampleCOM6, 9600 baud) to continuously receive accelerometer data from the microcontroller.

  - **Step Detection Algorithms:**  
    Two detection strategies are implemented:
    1. **Simple Difference-Based Detection:** Marks steps based on changes between consecutive samples.
    2. **Advanced Local Maxima Detection:** Utilizes a sliding window to detect peaks, enhanced by applying High-Pass (HPF) and Bandpass (BPF) filters to isolate running and walking frequencies, respectively. Adaptive thresholds and minimum sample distances further refine detection accuracy.

  - **Live Visualization:**  
    Six subplots display raw sensor data, filtered signals, and detection markers. The final subplot merges HPF and BPF outputs to provide a comprehensive view of the detected steps.

  - **Feedback Loop:**  
    After computing step detections, MATLAB send messages ("WALK++" or "RUN++") back to the microcontroller via UART, completing a real-time feedback cycle.


#### **Different levels of acceleration processing**
![Preview](MATLAB/MeasurementsWithTitles.png)
*Screenshot 1: Real-time step detection visualization in MATLAB.*


## **How to Use**
1. **Flash the firmware** onto the **FRDM-KL05Z** microcontroller.
2. **Connect the microcontroller to the PC** via a serial (UART) connection.
3. **Run the MATLAB script** to start real-time step detection and visualization.


## **Features**
- **High Performance:**  
  Optimized delays (100 ms, achieving a 10 Hz sampling rate) and efficient interrupt-driven execution ensure real-time processing even under intensive sensor data loads.

- **Interrupt-Driven Design:**  
  - **Button Interrupt (PORTA_IRQHandler):** Triggered on a falling edge on PTA11, this ISR resets the step counters and updates the LCD to notify the user.
  - **UART Interrupt (UART0_IRQHandler):** Activated upon receiving data via UART, this ISR processes incoming messages, updates step counters, and refreshes the LCD display.

- **Scalability and Efficient Resource Management:**  
  The modular architecture and use of manufacturer libraries reduce manual register manipulation, making the code easily adaptable to other devices or additional peripherals.
