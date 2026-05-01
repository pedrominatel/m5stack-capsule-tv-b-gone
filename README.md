# M5Stack Capsule TV-B-Gone

This project is a **TV-B-Gone** implementation for the [M5Stack Capsule](https://docs.m5stack.com/en/core/M5Capsule) (ESP32-S3). It uses the [TV-B-Gone component](https://components.espressif.com/components/pedrominatel/tv_b_gone) from the [ESP-IDF Component Registry](https://components.espressif.com/) to send IR power-off signals to a wide range of TVs.

It is inspired by the original [TV-B-Gone project by Mitch Altman](https://www.tvbgone.com/).

![M5Stack Capsule](https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/products/core/M5Capsule/4.webp)

## About TV-B-Gone

TV-B-Gone is an open-source project created by [Mitch Altman](https://www.tvbgone.com/) that uses infrared (IR) to send power-off codes to televisions. When activated, it cycles through hundreds of IR codes, turning off virtually any TV it points at.

## Hardware

- [M5Stack Capsule](https://docs.m5stack.com/en/core/M5Capsule) (ESP32-S3)
- Built-in IR LED on the M5Stack Capsule
- Built-in RGB LED used for run-state indication
- Wake button used to boot the board and trigger the transmission sequence

## Pin Map

![M5Stack Capsule Pin Map](https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/496/K129_CAPSILE-pin-sticker.png)

Default GPIO assignments in this project:

- `HOLD`: `GPIO46`
- Wake button: `GPIO42`
- IR LED TX: `GPIO4`
- RGB LED: `GPIO21`

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
3. Optionally adjust the Capsule GPIOs in `idf.py menuconfig` under `M5Capsule TV-B-Gone Configuration`.
4. Build and flash the project:
   ```bash
   idf.py set-target esp32s3
   idf.py build flash monitor
   ```

## Usage

Press the Capsule wake button once to power on the board and start the transmission flow. The firmware asserts the configured `HOLD` GPIO during boot so the board stays on while the sequence is running.

Working mode:

- One button press boots the device and starts the TV-B-Gone run.
- The RGB LED turns blue while EU codes are being sent.
- The RGB LED turns red while NA/US codes are being sent.
- After all codes finish, the RGB LED turns green for 5 seconds.
- After the green indication, the firmware releases `HOLD` and powers the board off.

## License

This project is licensed under the [Apache License 2.0](LICENSE).
