# Implementation Plan - Manual Inventory Adjustment

Update the filament inventory with the physical measurement values provided by the user.

## Proposed Changes

### [Component] Database (db.json)

#### [MODIFY] [db.json](file:///C:/project/private_projects/35_Projects/3D%EC%B6%9C%EB%A0%A5_%EC%83%9D%EC%82%B0%EA%B4%80%EB%A6%AC%EC%82%AC%EC%9D%B4%ED%8A%B8/3Dprint_MEM_forLocal_05/40_Software/data/db.json)
- Update `remaining_weight_g` and `updated_at` for the following batches:
    - A1: 300g
    - A2: 350g
    - A3: 300g
    - A4: 700g
- Set `updated_at` to `2026-02-13 09:00:00` (representing today's morning).

## Verification Plan

### Manual Verification
- Verify that the inventory list on the dashboard reflects the new weights.
- Confirm that the "Estimated Stock" calculation (which now uses start_time) correctly handles production data relative to this 09:00 measurement.
