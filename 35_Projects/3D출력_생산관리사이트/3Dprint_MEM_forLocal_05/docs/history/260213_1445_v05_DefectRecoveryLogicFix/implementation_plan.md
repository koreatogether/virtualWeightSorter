# Enhancement: Defect Recovery Alert & Filament Verification

Enhance the user experience by adding a filament re-check warning to recovery alerts and ensuring filament requirements for recovery runs are accurately reflected in the inventory status.

## Proposed Changes

### [Component] Frontend Logic

#### [MODIFY] [main.js](file:///C:/project/private_projects/35_Projects/3D출력_생산관리사이트/3Dprint_MEM_forLocal_05/40_Software/static/js/main.js)
- Update `renderDefectRecoveryAlerts` to include the message "※ 필라멘트 잔량을 다시 확인하세요!" (Please re-check filament stock).
- Verify that `renderInventory` correctly sums the filament requirements for all remaining production (including newly added recovery schedules).

## Verification Plan

### Manual Verification
- Manually trigger a defect recovery alert (by editing `db.json` temporarily if needed) and check the message.
- Observe the "Needed" (필요) quantity in the inventory table before and after adding a recovery schedule to confirm it covers the target gap.
