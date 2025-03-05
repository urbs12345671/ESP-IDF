This project (dht11_with_components) is designed to measure temperature and humidity using an DHT11 sensor with an ESP32 microcontroller. 

## How to use example
1 Open project folder in Visual Studio Code with ESP-IDF extension (i.e. v5.4). 
2 Build the project
3 Flash to ESP32 device
4 Use ESP-IDF monitor feature to see the output

## Folder contents
The project (dht11_with_components) contains a source file in C language and it is located in folder [main].  The component project (dht11) located under components folder encapsulates the main capabilities of a DHT11 sensor.

ESP-IDF projects are built using CMake. The projects' build configuration is contained in each `CMakeLists.txt`file that provide set of directives and instructions describing the project's source files and targets (executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── .vscode
├── components
│   └── dht11             Component project with command "idf.py create-component -C components dht11" in ESP-IDF Terminal
├── main
│   ├── CMakeLists.txt
│   └── main.c
│   └── .gitignore
├── CMakeLists.txt
└── README.md             This is the file you are currently reading 
└── version.txt
```
Additionally, the project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.

DHT11 is a basic, low-cost digital temperature and humidity sensor. It uses a capacitive humidity sensor and a thermistor to measure the surrounding air and outputs a digital signal on the data pin. Here's a breakdown of how it works:

Key Components of the DHT11
1 Capacitive Humidity Sensor:
    Measures relative humidity by detecting changes in capacitance caused by moisture in the air.
    The sensor has a moisture-holding substrate sandwiched between two electrodes. As humidity changes, the substrate absorbs or releases water vapor, altering the capacitance.

2 Thermistor (Temperature Sensor):
    A thermistor measures temperature by detecting changes in electrical resistance as the temperature fluctuates.
    The DHT11 uses a Negative Temperature Coefficient (NTC) thermistor, meaning its resistance decreases as temperature increases.
3 Microcontroller:
    The DHT11 has a built-in microcontroller that processes the analog signals from the humidity and temperature sensors and converts them into a digital signal.

How It Works
1 Power-Up:
    The DHT11 requires a power supply of 3.3V to 5V. When powered, it initializes and waits for a signal from the host microcontroller to start communication.

2 Communication Protocol:
    The DHT11 uses a single-wire bidirectional communication protocol. The host microcontroller sends a start signal to the DHT11, which then responds with a 40-bit data packet containing humidity and temperature readings.
    The data packet consists of:
        16 bits for humidity (integer and decimal parts).
        16 bits for temperature (integer and decimal parts).
        8 bits for a checksum to verify data integrity.
3 Data Transmission:
    The DHT11 sends data by pulling the data line low and high in specific patterns to represent 0s and 1s.
    The host microcontroller reads these signals and decodes them into humidity and temperature values.

Key Specifications
    Humidity Range: 20% to 80% (±5% accuracy).
    Temperature Range: 0°C to 50°C (±2°C accuracy).
    Sampling Rate: Once per second (1Hz).
    Operating Voltage: 3.3V to 5V.
    Output: Digital signal (no analog-to-digital converter required).

Limitations
    The DHT11 is less accurate and has a narrower measurement range compared to its more advanced sibling, the DHT22.
    It is slower and less suitable for applications requiring high precision or fast sampling.

Typical Applications
    Weather stations.
    Home automation systems.
    Environmental monitoring.
    Hobbyist projects.