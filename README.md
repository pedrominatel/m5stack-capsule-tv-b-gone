# M5Stack Capsule TV-B-Gone

This project is a **TV-B-Gone** implementation for the [M5Stack Capsule](https://docs.m5stack.com/en/core/M5Capsule) (ESP32-S3). It uses the [TV-B-Gone component](https://components.espressif.com/components/pedrominatel/tv_b_gone) from the [ESP-IDF Component Registry](https://components.espressif.com/) to send IR power-off signals to a wide range of TVs.

## About TV-B-Gone

TV-B-Gone is an open-source project that uses infrared (IR) to send power-off codes to televisions. When activated, it cycles through hundreds of IR codes, turning off virtually any TV it points at.

## Hardware

- [M5Stack Capsule](https://docs.m5stack.com/en/core/M5Capsule) (ESP32-S3)
- Built-in IR LED on the M5Stack Capsule

## Requirements

- [ESP-IDF](https://github.com/espressif/esp-idf) v5.0 or later
- [TV-B-Gone component](https://components.espressif.com/components/pedrominatel/tv_b_gone) (from the ESP-IDF Component Registry)

## Getting Started

1. Install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html).
2. Clone this repository:
   ```bash
   git clone https://github.com/pedrominatel/m5stack-capsule-tv-b-gone.git
   cd m5stack-capsule-tv-b-gone
   ```
3. Build and flash the project:
   ```bash
   idf.py set-target esp32s3
   idf.py build flash monitor
   ```

## Usage

Once flashed, the device will start sending TV power-off IR codes automatically. Point the M5Stack Capsule at a TV and it will cycle through all codes until the TV turns off.

## License

This project is licensed under the [Apache License 2.0](LICENSE).
