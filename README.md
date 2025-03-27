# 🚀 DALI LED Controller with Raspberry Pi 4 & ESP32

## 📌 Overview
This project implements **DALI LED control** using **ESP32** as a DALI LED driver and **Raspberry Pi 4** as the command unit.  
The Raspberry Pi sends user commands via a **custom kernel driver**, and ESP32 interprets these commands to control LEDs.  

This project was developed with **a cross-compiled kernel on Ubuntu**, then the DALI driver module was built using that kernel for the **Raspberry Pi 4 (ARM64)**.  

---

## 🛠 Project Components
### 🔹 1. **Kernel Module (\`my_DALI_drv.ko\`)**
- A **Linux kernel module** compiled on **Ubuntu (host machine)** using **cross-compilation** for **Raspberry Pi 4 (ARM64)**.  
- The driver creates a character device \`/dev/my_DALI_drv\` to send and receive DALI commands.  

### 🔹 2. **User-Space Applications**
- **C Program (\`RPi_DALI_app.c\`)**: Sends commands via \`/dev/my_DALI_drv\`.  
- **Python GUI (\`Raspi4_DALI_Controller_int.py\`)**: Provides a graphical interface for sending commands.  

### 🔹 3. **ESP32 Firmware (\`dali_led.ino\`)**
- Replaces a physical DALI controller by emulating DALI LED control logic.  
- Receives commands from Raspberry Pi via GPIO and controls LEDs accordingly.  

---

## 🏗 Building the Kernel Module
### 🔹 **Step 1: Install Cross-Compiler on Ubuntu (Host)**
```bash
sudo apt update
sudo apt install gcc-aarch64-linux-gnu make -y
```

### 🔹 **Step 2: Clone the Repository**
```bash
git clone https://github.com/HuynhDucRio/DALI_LED_rpi4.git
cd DALI_LED_rpi4
```

### 🔹 **Step 3: Cross-Compile Kernel for Raspberry Pi 4**
```bash
export KERNEL_SRC=/home/duc/Desktop/linux
make -C \$KERNEL_SRC ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=\$(pwd) modules
```

### 🔹 **Step 4: Transfer and Load Kernel Module on Raspberry Pi 4**
On **Ubuntu Host**, transfer the compiled \`.ko\` file:
```bash
scp my_DALI_drv.ko pi@raspberrypi.local:/home/pi/DALI_int/
```
On **Raspberry Pi**, load the module:
```bash
sudo insmod /home/pi/DALI_int/my_DALI_drv.ko
lsmod | grep dali  # Check if the module is loaded
```
To remove the module:
```bash
sudo rmmod my_DALI_drv
```

---

## 🎮 Running User-Space Applications
### **1. Using C Program**
```bash
gcc RPi_DALI_app.c -o dali_app
sudo ./dali_app
```
Enter **DALI address & data** in \`xxxxxx\` format (e.g., \`255254\`).  
The program transmits data to ESP32, which executes LED control.

### **2. Using Python GUI**
```bash
python3 Raspi4_DALI_Controller_int.py
```
- Enter **DALI address** and **data** in the respective fields.  
- Click **Send Command** to transmit data to ESP32.  
- Click **Receive Response** to read back data from the system.

### **3. Flashing ESP32 Firmware**
1. Open **Arduino IDE** and install the **ESP32 board package**.  
2. Connect ESP32 to your computer via USB.  
3. Open **\`dali_led.ino\`**.  
4. Select **ESP32 board** and correct COM port.  
5. Click **Upload** to flash the firmware.  
6. Open **Serial Monitor** to check communication logs.  

ESP32 listens for signals from Raspberry Pi and executes LED control commands.  

---

## 📬 Contact
📧 **Email**: ducdep102@gmail.com  

## 📜 License
This project is licensed under the **MIT License** - feel free to use and modify! 🎉  
EOF
