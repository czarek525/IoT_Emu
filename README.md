# IoT Emulator

## Description

The IoT Emulator is developed in C++ using modern and advanced techniques such as:
- Multithreading
- Factory pattern for object creation
- Multi-connection and multi-handler socket programming (TCP/UDP)
- Smart pointers
- Support for C++11
- CMake for building the project
- Object-oriented programming paradigm
- Optimized for lowest computational complexity

Additionally, a Python program is provided to validate the emulator and generate basic statistics by analyzing logs.

## Table of Contents

- [Requirements](#requirements)
  - [System Requirements](#system-requirements)
  - [Programming Language](#programming-language)
  - [Automatic Compilation](#automatic-compilation)
  - [Event Relay](#event-relay)
- [Installation](#installation)
  - [Windows](#windows)
    - [Eclipse Paho MQTT C](#eclipse-paho-mqtt-c)
    - [Eclipse Paho MQTT C++](#eclipse-paho-mqtt-c++)
  - [Linux](#linux)
    - [Eclipse Paho MQTT C++](#eclipse-paho-mqtt-c++)
- [Usage](#usage)
  - [Compile the Project](#compile-the-project)
  - [Run the Project](#run-the-project)

## Requirements

### System Requirements

- **Operating System**: Windows, Linux

### Programming Language

- **C++11**:
  - **MSVC (Windows)**: [Visual Studio C++](https://visualstudio.microsoft.com/downloads/)
  - **GCC (Linux)**: [Install GCC](https://www.geeksforgeeks.org/how-to-install-gcc-compiler-on-linux/)

- **Libraries**:
  - [`Eclipse Paho MQTT C`](https://github.com/eclipse/paho.mqtt.c)
  - [`Eclipse Paho MQTT C++`](https://github.com/eclipse/paho.mqtt.cpp)

### Automatic Compilation

- **CMake**

### Event Relay

- **MQTT Broker**: Example: [Mosquitto](https://mosquitto.org/)

## Installation

### Windows

#### Eclipse Paho MQTT C

1. **Download the library**:
   - Clone the repository:
     ```bash
     git clone https://github.com/eclipse/paho.mqtt.c.git
     ```
   - Alternatively, you can download the ZIP file from the repository and extract it.

2. **Install the library using CMake**:
   - **Windows**:
     1. Create a folder in `C:/PahoC`.
     2. Start the CMake GUI and configure it as follows (for the build path, it’s recommended to create a separate folder):
        
        ![CMake GUI Setup](https://github.com/user-attachments/assets/2d0feb8e-58e5-40d8-861b-eeab61109b63)

     3. Click **Generate** and then **Open Project**.
     4. Right-click on `INSTALL` and build the project.

        ![CMake Build](https://github.com/user-attachments/assets/39319eb4-ae5c-492f-b278-397334167511)

#### Eclipse Paho MQTT C++

1. **Download the library**:
   - Clone the repository:
     ```bash
     git clone https://github.com/eclipse/paho.mqtt.cpp.git
     ```
   - Alternatively, download the ZIP file and extract it.

2. **Install the library using CMake**:
   - **Windows**:
     1. Create a folder in `C:/PahoCPP`.
     2. Start the CMake GUI and configure it as follows (for the build path, it’s recommended to create a separate folder):
        
        ![CMake GUI Setup](https://github.com/user-attachments/assets/c9c3009d-2174-4011-9862-e538ebc9a574)

     3. Click **Generate** and then **Open Project**.
     4. Right-click on the solution and build it, similar to how it’s done in Eclipse Paho MQTT C.

### Linux

#### Eclipse Paho MQTT C++

1. **Install the library**:
    ```bash
    git clone https://github.com/eclipse/paho.mqtt.cpp.git
    cd paho.mqtt.cpp
    git submodule init
    git submodule update
    cmake -Bbuild -H. -DPAHO_WITH_MQTT_C=ON
    sudo cmake --build build/ --target install
    ```

### MQTT Broker

To ensure the emulator functions correctly, an event relay for output and environment events is needed. For testing purposes, **Mosquitto** can be used. [Download Mosquitto](https://mosquitto.org/download/).

## Usage
### Compile project

  ```bash
  git clone https://github.com/czarek525/IoT_Emulator.git
  cd IoT_Emulator
   //For Windows u can use parameter -G "Visual Studio 17 2022
  cmake -Bbuild -H. 
  cmake --build build/
  ```
### Run Project

Program required 2 arguments when first is time starting emulator, and second is path to folder with definition of IoT component(with rcr files.).
Remember that before start program is need to start mosquitto.
    
  ```bash
  cd build/Debug
```
Start the emulator in 2 seconds and path to rcr folder
  ```bash
  ./IoT_Emulator.exe 2 ../../rcr 
  ```
Also you can write in this form,
    
Start the emulator at 20:00:00 and path to rcr folder
  ```bash
  ./IoT_Emulator.exe 20:00:00 ../../rcr 
  ```
To end program is need to write "q" and this terminate program, wait untill close every sockets  and output file with logs create in the same directory.

<img width="364" alt="image" src="https://github.com/user-attachments/assets/7c7ef730-36c1-49a5-939a-ffc7a1bfbeed">

To analise logs, is need to start program main.py:
  ```bash
  cd ../python_test/
  pip install -r .\requirements.txt
  python .\main.py
  ```

![image](https://github.com/user-attachments/assets/a235ab7a-4bed-49ee-868d-aafd0f3c6dc6)


Click Analise shows this messega with statistic for current zoom:


![image](https://github.com/user-attachments/assets/30cefc92-2e67-48d8-a98a-e457bb5d0da4)



