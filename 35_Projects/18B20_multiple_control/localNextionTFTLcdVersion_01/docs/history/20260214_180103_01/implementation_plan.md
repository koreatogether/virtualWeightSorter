# Source Code Migration from serialVersion_02

Migration of 21 source files from the `serialVersion_02` project to the `localNextionTFTLcdVersion_01` project to serve as the baseline for Nextion display integration.

## Proposed Changes

### [Component Name] Firmware Source

#### [NEW] [Source Files](file:///E:/project/35_Projects/18B20_multiple_control/localNextionTFTLcdVersion_01/src)

- Copy `App.cpp`, `App.h`, `App_ID.cpp`, `App_Resolution.cpp`, `App_Threshold.cpp`
- Copy `Communicator.cpp`, `Communicator.h`, `DS18B20_Sensor.cpp`, `DS18B20_Sensor.h`
- Copy `DataProcessor.cpp`, `DataProcessor.h`, `MemoryUsageTester.cpp`, `MemoryUsageTester.h`
- Copy `SerialUI.cpp`, `SerialUI.h`, `Utils.cpp`, `Utils.h`, `config.h`, `exclude_memory_analysis.h`
- Copy `main.cpp`, `test_validation.cpp`

Total 21 files will be moved.

## Verification Plan

### Automated Tests
- Verify file counts in the target `src` directory.

### Manual Verification
- Confirm that the project can be opened and files are visible in the IDE.
