# Plan: Improve Sensor Visibility and Diagnostics

The system correctly detects sensors physically but filters them in the UI if they don't have a valid ID (1-8) assigned in their hardware memory (User Data). This confuses users when sensors are new or have been reset.

## Proposed Changes

### [SerialUI]
- **[MODIFY] [SerialUI.cpp](file:///e:/project/35_Projects/18B20_multiple_serial_control/serialVersion_02/30_Firmware/src/SerialUI.cpp)**
    - Update `displaySensorTable()` and `printSensorReport()` to check for invalid/unassigned sensors using `_dp->hasInvalidSensors()`.
    - If unassigned sensors are found, call `_dp->printInvalidSensorTable()` to show them clearly.
    - This ensures users see their sensors immediately even if IDs aren't set yet.

### [DataProcessor]
- **[MODIFY] [DataProcessor.cpp](file:///e:/project/35_Projects/18B20_multiple_serial_control/serialVersion_02/30_Firmware/src/DataProcessor.cpp)**
    - Refine `printInvalidSensorTable()` to be more descriptive (e.g., "Unassigned/Invalid ID Sensors Found").

### [App]
- **[MODIFY] [App.cpp](file:///e:/project/35_Projects/18B20_multiple_serial_control/serialVersion_02/30_Firmware/src/App.cpp)**
    - Update `initialize18b20()` to provide better feedback about *all* detected sensors, regardless of their ID state.

## Verification Plan

### Automated Tests
- Build verification using `pio run`.

### Manual Verification
- Check if the "--- Connected Sensors (Address List) ---" table appears when sensors have ID 0.
- Verify that assigning IDs moves sensors from the "Invalid" list to the main "Status" table.
