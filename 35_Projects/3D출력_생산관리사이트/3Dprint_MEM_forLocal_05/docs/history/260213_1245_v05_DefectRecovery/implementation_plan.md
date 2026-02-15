# Implementation Plan - Defect Recovery Alert System

Ensure that units lost to defects are never forgotten by automatically identifying production shortages and providing a quick-schedule mechanism.

## Proposed Changes

### [Component] UI Structure (index.html)

#### [MODIFY] [index.html](file:///C:/project/private_projects/35_Projects/3D%EC%B6%9C%EB%A0%A5_%EC%83%9D%EC%82%B0%EA%B4%80%EB%A6%AC%EC%82%AC%EC%9D%B4%ED%8A%B8/3Dprint_MEM_forLocal_05/40_Software/templates/index.html)
- Insert `<div id="defect-recovery-container"></div>` at the top of the `#schedule` section.

### [Component] Styling (style.css)

#### [MODIFY] [style.css](file:///C:/project/private_projects/35_Projects/3D%EC%B6%9C%EB%A0%A5_%EC%83%9D%EC%82%B0%EA%B4%80%EB%A6%AC%EC%82%AC%EC%9D%B4%ED%8A%B8/3Dprint_MEM_forLocal_05/40_Software/static/css/style.css)
- Add styling for `.recovery-alert`:
    - Background: `var(--conflict-bg)` (orange-ish).
    - Border: 1px solid `var(--warning-color)`.
    - Padding, border-radius, and flex layout for the message and action button.

### [Component] Frontend Logic (main.js)

#### [MODIFY] [main.js](file:///C:/project/private_projects/35_Projects/3D%EC%B6%9C%EB%A0%A5_%EC%83%9D%EC%82%B0%EA%B4%80%EB%A6%AC%EC%82%AC%EC%9D%B4%ED%8A%B8/3Dprint_MEM_forLocal_05/40_Software/static/js/main.js)
- Implement `renderDefectRecoveryAlerts(orders, schedules)`:
    - For each order, calculate: `Shortage = Target - (ActualTotal + FuturePlannedTotal)`.
    - If `Shortage > 0` AND the order has defects (`defect_quantity > 0`), display an alert.
- Implement `quickScheduleReplacement(orderId, qty)`:
    - Pre-fill the schedule form with the `orderId` and `qty`.
    - Scroll the user to the schedule form.
    - Open the date picker automatically.

## Verification Plan

### Manual Verification
1. **Shortage Detection**: Record a defect for an order. Verify that an orange alert appears stating exactly how many more units are needed.
2. **Planned Mitigation**: Add a *new* planned schedule for the missing amount. Verify that the alert disappears (even if not yet produced, it is now "planned").
3. **Quick Action**: Click the action button on the alert and verify the form is pre-filled correctly.
