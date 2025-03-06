# ledc_buzzer

LEDC" on an ESP32 stands for "LED Control" and refers to a dedicated peripheral on the chip that allows precise control of LEDs by generating Pulse Width Modulation (PWM) signals, essentially enabling you to adjust the brightness of LEDs by changing the duty cycle of the PWM signal; it's a key component for controlling LED intensity on ESP32 projects. 

In audio, LEDC essentially acts as a basic digital-to-analog converter (DAC) for simple audio applications. 


## Folder contents

The project **ledc_buzzer** contains three source files in C language in folder [main].

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
