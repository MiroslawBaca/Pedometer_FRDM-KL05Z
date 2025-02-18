# **Step Counter/Pedometer using FRDM-KL05Z & MATLAB**

This repository contains a real-time **step counting system** based on the **FRDM-KL05Z** microcontroller and **MATLAB** for data processing and visualization.

## **Project Overview**
### **FRDM-KL05Z (C++ Code)**
- Reads acceleration data from a **3-axis accelerometer** via I2C.
- Sends real-time data over **UART** to a connected PC.
- In progress

### **MATLAB (Data Processing & Visualization)**
- Receives accelerometer data via **serial communication (UART)**.
- Implements **step detection algorithms**:
  1. **Basic threshold-based detection** (simple peak detection).
  2. **MATLAB's `findpeaks`-based detection** (for refined peak identification).
- **Live visualization** of acceleration, magnitude, and detected steps in real-time.

#### **Features**
![Preview](MATLAB/MeasurementsWithTitles.png)
*Screenshot 1: Real-time step detection visualization in MATLAB.*




## **How to Use**
1. **Flash the firmware** onto the **FRDM-KL05Z** microcontroller.
2. **Connect the microcontroller to the PC** via a serial (UART) connection.
3. **Run the MATLAB script** to start real-time step detection and visualization.
