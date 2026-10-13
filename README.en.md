[Türkçe](README.md) | **English**

# ESP32-S3-Controlled Buck Converter

This project controls a synchronous buck converter with an ESP32-S3, reducing a 12 V DC input to approximately 5 V.

The ESP32-S3 generates a 20 kHz PWM signal that is applied to the power MOSFETs through an IR2184 gate driver. The output voltage is monitored through the board's measurement circuit and displayed on a web interface running over a local Wi-Fi network.

## System Architecture

![ESP32-S3-controlled buck converter system architecture](system_architecture.png)

The ESP32-S3:

- Generates the PWM signal
- Reads VSENSE_OUT with its ADC
- Calculates the output voltage
- Runs the web server
- Applies duty cycle commands from the user

## Main Features

- 20 kHz PWM generation
- 10-bit PWM resolution
- Manual duty cycle adjustment through the web interface
- PWM enable, disable, and emergency stop
- Live VOUT and VSENSE_OUT measurements
- ADC calibration
- Software overvoltage protection
- ESP32-S3 Wi-Fi Access Point mode
- Mobile and desktop browser support

## System Parameters

| Parameter | Value |
|---|---:|
| Input voltage | 12 V DC |
| Gate driver supply | 11.78 V |
| PWM frequency | 20 kHz |
| PWM resolution | 10 bits |
| Test duty cycle | 42% |
| Measured output voltage | 4.987 V |
| Measured VSENSE_OUT | 1.557 V |
| Firmware calibration factor | 3.175 |
| Test load | 100 Ω |

**Measurement consistency:** The 4.987 V / 1.557 V pair corresponds to a ratio of approximately 3.203. The firmware factor of 3.175 does not fully match this pair. The calibration session associated with the measurement pair needs to be verified; the current values are preserved here as separate records.

## Hardware Setup

![Buck converter hardware setup](hardware_setup.jpg)

### ESP32-S3 Connections

| ESP32-S3 | Buck board | Description |
|---|---|---|
| GPIO15 | IN | PWM through a 1 kΩ series resistor |
| GPIO4 | VSENSE_OUT | ADC through a 1 kΩ series resistor |
| 3V3 | 3V3 | Supply for the voltage measurement circuit |
| GND | GND | Common ground |

In the current version, `SD`, `ISENSE_OUT`, and `VS` are not connected to the ESP32-S3.

## Identified Problems and Solutions

### IR2184 VCC Connection

The IR2184 gate driver's VCC supply was found not to reach the board. A jumper was added to the VCC pin, providing approximately 11.78 V.

After this correction, the MOSFETs were driven and an output voltage dependent on duty cycle was obtained.

### Missing Voltage Measurement Components

R3, R4, and R6 were missing from the board. The voltage measurement stage was completed with the following values:

```text
R3 = 1 MΩ
R4 = 453 kΩ
R5 = Not populated
R6 = 0 Ω / solder bridge
```

With the initial resistor ratio, the op-amp output was limited to approximately 1.88 V. Changing R3 to 1 MΩ reduced the op-amp input level and stabilized the measurement system.

Final measurement:

```text
VOUT       = 4.987 V
VSENSE_OUT = 1.557 V
```

### Current Measurement Stage

Approximately 5.9 V was measured on ISENSE_OUT. This is not safe for an ESP32-S3 ADC input.

ISENSE_OUT must not be connected to the ESP32-S3 until the component in the current measurement circuit is verified.

## Test Results

Measurements obtained with a 100 Ω load:

| Duty cycle | Output voltage |
|---|---:|
| 30% | 3.50 V |
| 35% | 4.00 V |
| 40% | 4.70 V |
| 42% | 4.987 V |

## Serial Monitor Output

![ESP32-S3 serial monitor measurement](serial_monitor_measurement.png)

Output voltage is calculated as:

```cpp
VOUT = VSENSE_OUT * VSENSE_GAIN;
```

Calibration factor used:

```cpp
const float VSENSE_GAIN = 3.175;
```

## Web Control Interface

![Buck converter web interface](web_arayuz.png)

The ESP32-S3 creates its own Wi-Fi network and provides a local control panel without an external router.

```text
Wi-Fi name: Buck_Control
Password: buck12345
IP address: 192.168.4.1
```

Browser access address:

```text
http://192.168.4.1
```

The web interface includes:

- Live output voltage display
- Live VSENSE_OUT value
- Duty cycle slider
- Numerical duty cycle input
- PWM enable and disable
- Emergency stop
- Display of the 10-bit PWM register value
- Connection and system status indicators
- 5.60 V overvoltage protection

The panel is designed for manual fixed-PWM control. The user sets the duty cycle; the system does not adjust it automatically.

## Uploading the Firmware

1. Open [vsout_sonrevize.ino](vsout_sonrevize.ino) in Arduino IDE inside a sketch folder with the same name.
2. Select the ESP32-S3 board and the settings below. No external sensor or web library is used; Arduino, WiFi, and WebServer are included with the ESP32 core.
3. Compile the current code in an Arduino-ESP32 2.x environment. Changes are required for the LEDC functions removed in the 3.x core.
4. Upload the firmware, connect to `Buck_Control`, and open `http://192.168.4.1`.

[Arduino-ESP32 migration guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html)

## Arduino IDE Settings

```text
Board: ESP32S3 Dev Module
USB CDC On Boot: Enabled
Serial Monitor: 115200 baud
```

The project uses the Arduino-ESP32 2.x LEDC API:

```cpp
ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
ledcAttachPin(PIN_PWM, PWM_CHANNEL);
```

## Safety

- Use a current-limited laboratory power supply for initial tests.
- Do not apply more than 3.3 V to ESP32-S3 GPIO pins.
- ISENSE_OUT must not be connected to the ESP32-S3 in its current state.
- Do not power the IR2184 VCC line from the ESP32-S3's 3.3 V output.
- Start with a low duty cycle and increase it gradually.
- Software protection does not replace hardware protection.

## Results

The project achieved:

- Operation of the buck converter power stage
- Correction of the IR2184 VCC connection problem
- Completion of missing voltage measurement components
- Improvement of the op-amp operating range
- Calibration of ESP32-S3 ADC measurements
- Development of a local web control interface
- Approximately 5 V output at 42% duty cycle

## Future Improvements

- Correct the current measurement stage
- Add hardware overcurrent protection
- Implement closed-loop PI control
- Compare results with a PLECS model
- Run efficiency and temperature tests
- Develop a new PCB revision
- Add live charts and data logging to the web interface

## My Contribution

I inspected the non-working board, identified the gate driver supply issue and missing voltage measurement components, applied the hardware corrections, and measured the output at different duty cycles. I developed ESP32-S3 PWM generation, ADC acquisition, overvoltage shutdown, and the local web interface.

This work covers fault analysis, repair, and control firmware development; it does not claim authorship of the original buck board design. The test points shown are not closed-loop voltage regulation or efficiency tests.

## Repository Contents

- [Embedded firmware and web interface](vsout_sonrevize.ino)
- Images of the system architecture, laboratory setup, serial measurements, and web interface

[My other engineering projects](https://github.com/mertuzun41/Projeler/blob/main/README.en.md)

## Academic Information

**Academic advisor:** Assoc. Prof. Dr. Mehmet Dal  
**Institution:** Kocaeli University
