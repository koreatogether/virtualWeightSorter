# PlatformIO Configuration for Nextion Project (Specialized)

I have updated the `platformio.ini` file for the `localNextionTFTLcdVersion_01` project using the specialized template provided for Nextion-based ESP32-C3 projects.

## Changes Made

### [platformio.ini](file:///E:/project/35_Projects/18B20_multiple_control/localNextionTFTLcdVersion_01/platformio.ini)

- **Platform Version**: Locked to `espressif32@6.7.0` as per the specialized template.
- **Library Management**:
    - `lib_ldf_mode = deep+` enabled for robust nested dependency resolution.
    - `lib_extra_dirs` points to the user's global Arduino library path: `C:\Users\h\Documents\Arduino\libraries`.
    - `lib_ignore` excludes several standard libraries (SD, FreeRTOS, etc.) to prevent conflicts.
- **Monitor Speed**: `115200`.
- **Build Flags**: `-D ARDUINO_USB_CDC_ON_BOOT=1` for Serial debugging.
- **Libraries (lib_deps)**:
    - `paulstoffregen/OneWire@^2.3.7`
    - `milesburton/DallasTemperature@^3.11.0`
    - `itead/Nextion@^0.9.1`

## Final Configuration Result

```ini
; PlatformIO Project Configuration File for ESP32-C3 XIAO (Nextion Version)
; Specialized configuration based on user template for Nextion projects

[env:xiao_esp32c3_nextion_v01]
platform = espressif32@6.7.0
board = seeed_xiao_esp32c3
framework = arduino

; Serial Monitor options
monitor_speed = 115200
monitor_filters = 
    default
    time

; Upload options
upload_speed = 921600

; Library management
lib_ldf_mode = deep+
lib_extra_dirs = 
    C:\Users\h\Documents\Arduino\libraries

lib_ignore = 
    SD
    FreeRTOS
    WiFiNINA
    MQ7Sensor
    ArduinoMDNS

; Build options
build_flags = 
    -D ARDUINO_USB_CDC_ON_BOOT=1

; Source filter - Adjusted to point to src directory
build_src_filter = +<src/>

; Library dependencies (Added for project requirements)
lib_deps = 
    paulstoffregen/OneWire@^2.3.7
    milesburton/DallasTemperature@^3.11.0
    itead/Nextion@^0.9.1
```
