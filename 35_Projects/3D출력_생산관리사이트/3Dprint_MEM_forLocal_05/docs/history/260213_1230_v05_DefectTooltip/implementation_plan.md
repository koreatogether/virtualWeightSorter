# Implementation Plan - Defect Reason Tooltip

Enhance the production schedule UI by adding a hoverable tooltip that displays a detailed list of defect reasons when the user points to the defect count.

## Proposed Changes

### [Component] Styling (style.css)

#### [MODIFY] [style.css](file:///C:/project/private_projects/35_Projects/3D%EC%B6%9C%EB%A0%A5_%EC%83%9D%EC%82%B0%EA%B4%80%EB%A6%AC%EC%82%AC%EC%9D%B4%ED%8A%B8/3Dprint_MEM_forLocal_05/40_Software/static/css/style.css)
- Add `.tooltip` class for the container (relative position).
- Add `.tooltip-text` class for the hidden-by-default popup.
    - Style with: background color (matching accent), white text, padding, border-radius, box-shadow.
    - Implement visibility on hover with a smooth fade-in animation.
    - Ensure it handles multiple lines (list of reasons).

### [Component] Frontend (main.js)

#### [MODIFY] [main.js](file:///C:/project/private_projects/35_Projects/3D%EC%B6%9C%EB%A0%A5_%EC%83%9D%EC%82%B0%EA%B4%80%EB%A6%AC%EC%82%AC%EC%9D%B4%ED%8A%B8/3Dprint_MEM_forLocal_05/40_Software/static/js/main.js)
- Update `renderSchedules` to wrap the `불량: ${s.defect_quantity}` text in a `div` or `span` with the `.tooltip` class.
- Generate a formatted string of `defect_reasons` (e.g., joined by newlines or bullets) and place it inside the `.tooltip-text` child element.

## Verification Plan

### Manual Verification
1. **Hover Test**: Hover over a schedule with recorded defects. Verify that a styled box appears showing the reasons (e.g., "3개: 들뜸").
2. **Multi-Entry Test**: Add another defect to the same schedule. Verify that the tooltip now shows both entries.
3. **Empty Test**: Verify that schedules with 0 defects do not show the tooltip or any visual artifact.
