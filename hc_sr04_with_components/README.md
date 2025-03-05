This project (hc_sr04_with_components) is designed to measure distance using an HC-SR04 ultrasonic sensor with an ESP32 microcontroller. Here's a breakdown of its purpose and components:

Purpose:
This code enables an ESP32 to accurately measure distances (2cm-4m) using the HC-SR04 ultrasonic sensor, typical for applications like obstacle detection, level monitoring, or robotics.

Key Components:
Sensor Interface:
TRIG Pin (GPIO32): Generates a 10µs pulse to initiate ultrasonic burst.
ECHO Pin (GPIO33): Measures pulse width proportional to distance via edge detection.

MCPWM Capture Module:
Uses ESP32's Motor Control PWM capture to precisely time ECHO pulse edges (rising/falling).
Capture Timer: Clock-driven counter for timestamping edges.
Capture Channel: Configured for dual-edge detection on ECHO pin.

Interrupt Callback:
hc_sr04_echo_callback: Records timestamps on rising/falling edges, calculates pulse width in timer ticks, and notifies the main task.

Distance Calculation:
Converts timer ticks to microseconds using APB clock frequency.
Applies formula Distance = Pulse Width (µs) / 58 to derive centimeters (based on speed of sound).

Workflow:
Trigger Pulse: A 10µs pulse on TRIG starts the ultrasonic measurement.
Echo Capture: MCPWM captures rising (start) and falling (end) edges of ECHO pulse.
Pulse Width: Time difference between edges is computed in the ISR.
Notification: Main task processes pulse width, converts to distance, logs result, and repeats every 500ms.

Safety Features of the code includes:
    Input validation
    Resource cleanup
    Error propagation
    Bounds checking


## How to use example
1 Open project folder in Visual Studio Code with ESP-IDF extension (i.e. v5.4). 
2 Build the project
3 Flash to ESP32 device
4 Use ESP-IDF monitor feature to see the output

## Folder contents
The project (hc_sr04_with_components) contains a source files in C language and it is located in folder [main].  The component project (hc_sr04) located under components folder encapsulates the main capabilities of a HC-SR04 ultrasonic sensor.

ESP-IDF projects are built using CMake. The projects' build configuration is contained in each `CMakeLists.txt`file that provide set of directives and instructions describing the project's source files and targets (executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── .vscode
├── components
│   └── hc_sr04           Component project with command "idf.py create-component -C components hc_sr04" in ESP-IDF Terminal
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


Below a sequence diagram to show how HS-SR04 sensor works with a microcontroller (like ESP32) to measure distance . The main components involved are the Microcontroller (like ESP32), the HC-SR04 sensor, and the sound waves interacting with the target object.

Here is an outline of the steps:
1. Microcontroller sends a trigger pulse (10µs high signal).
2. HC-SR04 emits an 8-cycle ultrasonic burst at 40kHz.
3. The sound waves travel to the target and reflect back.
4. HC-SR04 detects the echo and sets the ECHO pin high for the duration it took.
5. Microcontroller measures the ECHO pulse width.
6. Convert the pulse duration into distance using the speed of sound.

HC-SR04 Operational Theory Diagram
+-------------------+          +-------------------+          +-------------------+
|  Microcontroller  |          |    HC-SR04        |          |    Target Object  |
|  (e.g., ESP32)    |          |  Ultrasonic Sensor|          |                   |
+-------------------+          +-------------------+          +-------------------+
         |                             |                             |
         | 1. Trigger Pulse (10µs)     |                             |
         |---------------------------> |                             |
         |                             |                             |
         |                             | 2. 40kHz Ultrasonic Burst   |
         |                             |---------------------------->|
         |                             |          (8 cycles)         |
         |                             |                             |
         |                             | 3. Sound Wave Reflection    |
         |                             |<----------------------------|
         |                             |                             |
         | 4. Echo Pulse (Duration =   |                             |
         |<--------------------------- |                             |
         |   Time-of-Flight ×2)        |                             |
         |                             |                             |
         | 5. Calculate Distance:      |                             |
         |    Distance = (Pulse Width  |                             |
         |    × Speed of Sound) / 2    |                             |
         |                             |                             |
+--------v---------+           +-------v--------+            +-------v--------+
| Trigger Signal   |           | Ultrasonic     |            | Sound Wave     |
| (Digital Output) |           | Transducer Pair|            | Reflection     |
+------------------+           +----------------+            +----------------+

Step-by-Step Explanation of the diagram:
1 Trigger Signal
    Microcontroller sends a 10µs HIGH pulse to TRIG pin
    Example code: gpio_set_level(TRIG_PIN, 1)
2 Ultrasonic Burst
    HC-SR04 emits 8 cycles of 40kHz ultrasonic waves
    Frequency: 40kHz (inaudible to humans)
    Duration: ~200µs (8 cycles × 25µs/cycle)
3 Wave Propagation
    Sound travels at ~343m/s (at 20°C)
    Formula: Distance = (Speed × Time) / 2
    Why divide by 2? The sound makes a round trip
4 Echo Detection
    ECHO pin goes HIGH when burst is transmitted
    ECHO pin stays HIGH until echo is detected
    Pulse width = Time between transmission and echo reception
5 Distance Calculation
    Convert pulse width (µs) to distance:
        Distance (cm) = Pulse Width (µs) / 58
        Distance (inches) = Pulse Width (µs) / 148

Timing Diagram:
TRIG Pin:  __|¯¯¯¯¯10µs¯|_________________________________
ECHO Pin:  ________________|¯¯¯¯¯¯¯¯¯¯Pulse Width¯¯¯¯¯¯¯|__
                           |<-------- Time-of-Flight ---->|

Key Characteristics:
Working Voltage: 5V DC
Measuring Angle: 15° cone
Range: 2cm-400cm (theoretical), 2cm-200cm (practical)
Resolution: 0.3cm
Update Rate: 10Hz (best used with 50-100ms delays between measurements)

This explains why the code:
Uses precise timing for the trigger pulse
Needs accurate pulse width measurement
Requires division by 58 for cm conversion
Includes timeout handling (>35ms ≈ 6m limit)   


The division by 58 in the HC-SR04 distance calculation is derived from the speed of sound and the need to convert time (measured in microseconds) into distance (centimeters). Here's the detailed breakdown:

Step 1: Speed of Sound
At room temperature (~20°C), the speed of sound in air is approximately 343 meters/second (or 34,300 cm/second).

Step 2: Time-to-Distance Relationship
The HC-SR04 measures the round-trip time for an ultrasonic pulse to travel to an object and echo back.
    Total distance = Distance to object × 2
    Time measured = Total time for the round trip.
Thus, the one-way distance is:
    Distance = (Speed of Sound × Time) / 2
 
Step 3: Unit Conversions
    Convert speed to cm/µs:

        34,300 cm/s = (34,300 cm / 1,000,000) cm/µs = 0.0343 cm/µs
    Include division by 2 (for round trip):
        Effective Speed = 0.0343 / 2 = 0.01715 cm/µs

    Distance formula:
        Distance (cm) = Pulse Width (µs) × 0.01715

Step 4: Simplify to Division by 58
Instead of multiplying by 0.01715, we take its reciprocal to use division:
    1 / 0.01715 = 58.3
Thus, the simplified formula becomes:
    Distance (cm) = Pulse Width (µs) * 58
 
Why 58 and Not 58.3?
    Practical approximation: The value is rounded for simplicity in code.
    Temperature sensitivity: The speed of sound varies slightly with temperature (e.g., at 25°C, it’s ~346 m/s, changing the divisor to ~58.8).
    The HC-SR04’s ±3mm accuracy tolerates this rounding.

Example Calculation
If the pulse width is 580 µs:
    Distance = 580 / 58 = 10 cm





