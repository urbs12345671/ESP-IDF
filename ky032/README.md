# KY-032 Obstacle Avoidance Sensor Component

ESP-IDF component for interfacing with the KY-032 infrared obstacle avoidance sensor. Provides both polling and interrupt-driven operation with power management capabilities.

Please see KY-032.png for image of the sensor and its schematic

## Features

- **Dual Mode Operation**
  - Polling mode for periodic checks
  - Interrupt-driven mode for immediate detection
- **Power Management**
  - EN pin control for low-power operation
  - Automatic enable on initialization
- **Flexible Configuration**
  - User-selectable GPIO pins
  - Adjustable interrupt types
- **Thread Safety**
  - FreeRTOS queue integration
  - Safe ISR handling

