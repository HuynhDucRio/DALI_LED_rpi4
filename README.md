# DALI LED Controller using ESP32 & Raspberry Pi 4

🚀 **A Custom DALI Communication System using ESP32 as a DALI LED Driver**

## 📌 Overview
This project uses **ESP32 as a DALI LED driver**, receiving commands from **Raspberry Pi 4** to control individual or grouped LED systems. Raspberry Pi handles user inputs and transmits signals to ESP32, which then executes the DALI protocol for LED control.

## 🌟 Features
- **ESP32 as DALI LED Driver**: Implements DALI protocol to control LED clusters.
- **Raspberry Pi as Command Unit**: Sends user-space commands to ESP32.
- **Custom Command Interface**: Allows brightness adjustment, group control, and real-time LED status updates.
- **Multiple Control Options**: Commands can be sent via **C program (`RPi_DALI_app.c`)**, **Python GUI (`Raspi4_DALI_Controller_int.py`)**, and **ESP32 firmware (`dali_led.ino`)**.

## 🛠️ Technologies Used
- **Hardware**: Raspberry Pi 4, ESP32, DALI-compatible LED drivers.
- **Software**: C (User-Space App), Python (Tkinter GUI), Embedded C (ESP32 Firmware), Arduino IDE, Linux Device Drivers.
- **Communication**: DALI Protocol, UART/SPI (for Raspberry Pi to ESP32 communication).

## 📷 Screenshots
![System Diagram](![image](https://github.com/user-attachments/assets/dca17d6d-386c-4a60-9bab-69119b5251b0)
*System architecture showing Raspberry Pi sending commands to ESP32 for LED control*

## 🚀 Getting Started
### 📌 Hardware Requirements
- **Raspberry Pi 4**
- **ESP32**
- **DALI LED Drivers & LED Modules**
- **Power Supply & Wiring Components**

### 🔧 Installation & Setup
1. **Clone this repository**
   ```bash
   git clone https://github.com/HuynhDucRio/DALI_LED_rpi4.git
   cd DALI_LED_rpi4
   ```
2. **Build and Install the Kernel Module**
   ```bash
   make
   sudo insmod dali_driver.ko
   ```
3. **Verify Driver is Loaded**
   ```bash
   lsmod | grep dali
   ```
4. **Remove Kernel Module (if needed)**
   ```bash
   sudo rmmod dali_driver
   ```
5. **Clean Build Files**
   ```bash
   make clean
   ```
6. **Control LEDs via User-Space Program**
   - **Using C Program (`RPi_DALI_app.c`)**:
     ```bash
     gcc RPi_DALI_app.c -o dali_app
     sudo ./dali_app
     ```
     - Enter **DALI address & data** in `xxxxxx` format (e.g., `255254`).
     - The program transmits data to ESP32, which executes LED control.
   
   - **Using Python GUI (`Raspi4_DALI_Controller_int.py`)**:
     ```bash
     python3 Raspi4_DALI_Controller_int.py
     ```
     - Enter the **DALI address** and **data** in the respective fields.
     - Click **Send Command** to transmit data to ESP32.
     - Click **Receive Response** to read back data from the system.
   
   - **Flashing Code to ESP32 using Arduino IDE**:
     1. Open **Arduino IDE** and install the **ESP32 board package**.
     2. Connect ESP32 to your computer via USB.
     3. Open the file `dali_led.ino`.
     4. Select **ESP32 board** and correct COM port.
     5. Click **Upload** to flash the firmware.
     6. Open **Serial Monitor** to check communication logs.
     7. ESP32 listens for signals from Raspberry Pi and executes LED control commands.

7. **Monitor Logs on Raspberry Pi**
   ```bash
   dmesg | tail -n 20
   ```

## 🔗 Project Demo & Repo
- **GitHub Repository**: [DALI_LED_rpi4](https://github.com/HuynhDucRio/DALI_LED_rpi4)

## 📬 Contact
📧 **Email**: ducdep102@gmail.com  

## 📜 License
This project is licensed under the **MIT License** - feel free to use and modify! 🎉
