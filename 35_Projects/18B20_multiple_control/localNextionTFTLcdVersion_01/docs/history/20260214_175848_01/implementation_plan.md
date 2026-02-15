# Create PlatformIO Configuration for Nextion Project

The goal is to create a `platformio.ini` file for the `localNextionTFTLcdVersion_01` project, using the `serialVersion_01` project as a reference for environment variables and base libraries.

## Proposed Changes

### [Component Name] 18B20_multiple_control

#### [MODIFY] [platformio.ini](file:///E:/project/35_Projects/18B20_multiple_control/localNextionTFTLcdVersion_01/platformio.ini)

- Update `platform` to `espressif32@6.7.0`.
- Add `lib_ldf_mode = deep+`.
- Add `lib_extra_dirs = C:\Users\h\Documents\Arduino\libraries`.
- Add `lib_ignore` section (SD, FreeRTOS, etc.).
- Update `build_src_filter` to follow the `src/main/...` pattern.
- Keep `monitor_speed = 115200`.
- Include `OneWire`, `DallasTemperature`, and `Nextion` in `lib_deps` (or check if they should be omitted as per template).

## Verification Plan

### Automated Tests
- Run `pio project init --dry-run` or similar (if possible) to check if the file is valid.
- Check if libraries are correctly specified by running `pio lib list` (requires PlatformIO CLI).

### Manual Verification
- The user can open the project in VS Code with PlatformIO and verify that the environment is correctly detected.
