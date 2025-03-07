# ESP-IDF TTP223 Touch Sensor Component

The TTP223 is a capacitive touch sensor that can be used to replace traditional buttons. It has a low power consumption and can be used for DC or AC applications. 

Applications 
    Detect and switch on external units using an external relay
    Commercial solid state solution to replace dome, membrane, and mechanical keypads

TTP223 Touch Sensor
    See "TTP223.jpg" include in the project

Capacitive touch switch circuit diagram
    See "Capacitive Touch Switch Circuit.jpg" include in the project    


This project contains a complete ESP-IDF component for interfacing with TTP223 capacitive touch sensors. Provides both polling and interrupt-driven operation modes with safe event queue handling.

## Features

- **GPIO Configuration**: Flexible input configuration with pull-up/pull-down support
- **Interrupt-Driven Operation**: Low-latency touch detection using GPIO interrupts
- **Event Queue System**: Thread-safe communication between ISR and application
- **Multiple Sensor Support**: Manage multiple TTP223 sensors simultaneously
- **Debouncing Included**: Built-in software debouncing for reliable detection

## Installation

This component is not registered in "The ESP Component Registry". You can download the component from Github and reference it in your project.  Please be sure to add `idf_component.yml` in your project and use the "override_path" as described in the ESP-IDF documentation.

### Using Component Manager
Add to your project's `idf_component.yml`:
```yaml
dependencies:
  ttp223:
    version: "*"
    override_path: "../../../"