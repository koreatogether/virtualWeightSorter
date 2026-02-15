# Walkthrough - Defect Recovery Alert System (v05)

I have implemented an intelligent monitoring system that ensures production shortages caused by defects are never overlooked.

## New Features

### [Component] Defect Recovery Alerts
- **Real-time Monitoring**: The system automatically calculates exactly how many more units are needed for an order, taking into account current performance, existing inventory, and already planned future runs.
- **Visual Warning**: If an order has recorded defects and is currently short of its target, a bright orange alert banner appears at the top of the **Production Schedule** section.
- **Quick Recovery Button**: Clicking the "복구 일정 추가" (Add Recovery Schedule) button automatically fills the scheduling form with the correct order and the exact missing quantity.

## Verification Results

### Manual Verification
1. **Shortage Detection**: If an order for 100 units has 10 units of defects, and total planned/actual units are only 90, the alert will appear showing a 10-unit shortage.
2. **Auto-Filling**: Clicking the alert's button correctly fills the form with the order name and quantity.
3. **Planning Resolution**: Once a new schedule is added to cover the shortage, the alert disappears immediately (as the units are now "planned").

The system now proactively assists you in meeting production targets despite unexpected defects.
