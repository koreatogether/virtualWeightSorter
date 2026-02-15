# progressLog.md - DS18B20 Resolution Management & UI Update (serialVersion_02)

## 개요
- **일시**: 2026-02-13 23:55:00 (5차 업데이트 - 최종)
- **목적**: 센서 해상도(9-12 bit) 변경 기능 추가 및 메인 테이블 시각화 개선.
- **주요 변경 사항**: 하드웨어 제어 메서드 추가, UI 메뉴 확장, 메인 테이블 "Res" 컬럼 추가.

## 진행 내역

### 1. 하드웨어 레이어 확장 (`DS18B20_Sensor`)
- `getResolution(address)`: 현재 설정된 해상도 비트값 읽기 기능 추가.
- `setResolution(address, bits)`: 특정 센서의 해상도 변경.
- `setResolution(bits)`: 모든 센서의 해상도를 일괄 변경.

### 2. UI 및 테이블 시각화 개선
- **메인 테이블 업데이트**: "Res" 컬럼을 추가하여 각 센서의 현재 해상도(9, 10, 11, 12)를 실시간으로 확인 가능하도록 개선.
- **메뉴 확장**: 메인 메뉴 5번에 `Resolution Management` 항목 추가.

### 3. 애플리케이션 로직 구현 (`App`)
- `RESOLUTION_MENU` 상태 머신 및 서브 상태(`RES_SELECT_SENSOR`, `RES_SELECT_BITS`) 구현.
- ID 부여 로직과 일관된 인터페이스로 개별/단체 해상도 설정 프로세스 완성.

### 4. 품질 검증 및 최적화
- **정적 분석**: `pio check`를 통해 신규 변수(`currentResSubState`, `selectedResolution`) 초기화 누락 확인 및 수정.
- **최종 빌드/업로드**: 모든 기능 통합 후 Seeed XIAO ESP32-C3 배포 완료.

## 최종 아티팩트 보관
- **현재 위치**: `docs/history/20260213_234000_05_Resolution_Feature/`
- **보관 항목**: `progressLog.md`, `task.md`, `implementation_plan.md`

---
*해상도 제어와 시각화 기능이 모두 통합되어, 센서 관리 시스템의 완성도가 한층 더 높아졌습니다.*
