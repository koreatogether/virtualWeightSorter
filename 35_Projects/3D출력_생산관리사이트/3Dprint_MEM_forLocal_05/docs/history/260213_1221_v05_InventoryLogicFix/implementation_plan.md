# Implementation Plan - Defect Management & Past Date Access

Unlock the ability to select past dates for production scheduling and implement a defect tracking system to accurately reflect filament consumption and production quality.

## Proposed Changes

### [Component] Frontend (main.js)

#### [MODIFY] [main.js](file:///C:/project/private_projects/35_Projects/3D%EC%B6%9C%EB%A0%A5_%EC%83%9D%EC%82%B0%EA%B4%80%EB%A6%AC%EC%82%AC%EC%9D%B4%ED%8A%B8/3Dprint_MEM_forLocal_05/40_Software/static/js/main.js)
- Remove `minDate: "today"` from `flatpickr` initialization to allow past date selection. [DONE]
- Update `calculateConsumedWeight` to use `sch.start_time` (production date) instead of `sch.created_at` (DB entry time) for comparison with the last inventory measurement time. This prevents double-counting of historical data entered after the physical measurement was taken.
- Update `renderSchedules` to add "Defect" button and display defect totals. [DONE]
- Implement `updateRunDefect(id)` and update consumption sum to include defects. [DONE]

### [Component] Backend (app.py)

#### [MODIFY] [app.py](file:///C:/project/private_projects/35_Projects/3D%EC%B6%9C%EB%A0%A5_%EC%83%9D%EC%82%B0%EA%B4%80%EB%A6%AC%EC%82%AC%EC%9D%B4%ED%8A%B8/3Dprint_MEM_forLocal_05/40_Software/app.py)
- Modify `update_schedule(id)` handler:
    - Handle `defect_quantity` by adding it to the existing `defect_quantity` value (cumulative).
    - Handle `defect_reason` by appending it to a local list of reasons for that schedule.
    - Ensure that defect recordings trigger a database write and are logged in `user_actions.log`.

## Verification Plan

### Automated Tests
- None available; manual verification required.

### Manual Verification
1. **Unlock Dates**: Open the schedule creation tool and verify that dates before Feb 13, 2026, can be selected.
2. **Defect Recording**:
    - Click "불량입력" on a schedule.
    - Enter a quantity (e.g., 2) and a reason (e.g., "Layer shift").
    - Verify that "불량: 2" appears in the schedule row.
    - Verify that recording another defect (e.g., 1) updates the total to 3.
3. **Filament Calculation**:
    - Check the filament inventory "Estimated Stock" (추정 잔량).
    - Record a defect and verify that the estimated stock decreases by `(defect_quantity * unit_weight_g)`.
4. **Order Status**: Verify that defects do **not** increase the `completed_quantity` of an order.
