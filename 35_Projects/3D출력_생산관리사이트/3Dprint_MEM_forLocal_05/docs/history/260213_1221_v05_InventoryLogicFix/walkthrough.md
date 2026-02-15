# Walkthrough - Defect Management & Past Date Access (v05)

I have implemented the requested features to unlock past date selection and introduce a defect management system.

## Changes Made

### [Component] Production Scheduling
- **Unlocked Past Dates**: Removed the restriction that prevented selecting dates before today. You can now schedule or adjust tasks for any date.

### [Component] Defect Management
- **Defect Input**: Added a "불량입력" (Record Defect) button to the schedule list.
- **Reason Tracking**: You can now record both the quantity of defects and the reason for them.
- **Cumulative Tracking**: Both "Performance" (실적) and "Defects" (불량) are now cumulative, allowing you to add data multiple times.

### [Component] Filament Calculations
- **Accurate Consumption**: The estimated filament stock now correctly subtracts both successful production items **and** defective items.
- **Double-Counting Prevention**: Changed the weight deduction logic to use the **production date** (`start_time`) instead of the database entry time. This ensures that historical production data entered after a physical inventory measurement does not cause a negative balance.
- **Order Status Protection**: Defective items are recorded for filament tracking but do **not** contribute to the "Completed Quantity" of an order.

## Verification Results

### Manual Verification
- **Date Picker**: Confirmed that Feb 12 and other past dates are selectable in the "Run Schedule" form.
- **Cumulative Inputs**: Verified that adding "2 defects" then "1 defect" results in "3 defects total" for a schedule.
- **Consumption Logic**: Verified that recording a defect immediately reflects in the "Estimated Stock" of the corresponding filament in the inventory.
