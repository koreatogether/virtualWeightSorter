# Data Schema (Updated for Printer Asset ID & Conflict Check)

## 1. 프린터 (Printers)
`db.json` 내 `printers` 배열

```json
{
  "id": "uuid",
  "model": "P2SC",
  "asset_id": "P2SC-101",     // (New) 사용자가 관리하는 고유 식별 번호
  "purchase_date": "2026-02-04",
  "status": "idle | busy"
}
```

## 2. 중복 체크 로직 (Conflict Detection)
스케줄 생성(단일/자동) 시 다음 조건을 검사:
- 조건: `New Schedule Date` == `Existing Schedule Date` AND `New Printer ID` == `Existing Printer ID`
- 동작:
    - API: 경고 메시지 반환 (`warning`: "Printer busy on 2026-02-11")
    - UI: 해당 스케줄 행의 배경색을 주황색(`orange`)으로 표시하고 툴팁으로 "중복 할당됨" 표시.
