# 프로젝트 구조 재편 및 추상화 제안서

## 1. 개요 (Overview)
본 문서는 현재 혼재되어 있는 프로젝트 파일들을 기능적 복잡도와 목적에 따라 계층적으로 분류하여, **확장 가능하고 직관적인 폴더 구조**로 재편하기 위한 제안입니다.

**핵심 목표:**
*   **기능 단위 격리:** 순수 모니터링 기능과 비즈니스 로직(활용)의 분리
*   **검색 용의성:** 넘버링에 의존하지 않는 의미 기반의 구조
*   **확장성:** 프로젝트가 100개 이상으로 늘어나도 관리가 용이한 구조 (Monorepo 스타일)

---

## 2. 제안하는 폴더 구조 (Proposed Structure)
현재의 `01_`, `02_` 식의 순차적 나열 대신, **'기능의 깊이'**와 **'도메인'**을 축으로 하는 대분류 구조를 제안합니다.

```text
E:\PROJECT\
│
├── 00_Management/           # 프로젝트 관리, 아이디어, BOM 등 공통 자원
│   ├── BOM_Lists/           # (구 10_BOM_list)
│   ├── Ideas_Backlog/       # (구 앞으로도전하고싶은목록.md 등)
│   └── Standards/           # (구 프로그래밍규칙.txt)
│
├── 10_Components_Basic/     # [Level 1] 센서/액추에이터 단일 제어 (Hello World 수준)
│   ├── Sensors/
│   │   ├── DS18B20_Basic/   # (기본 온도 읽기)
│   │   ├── DHT22_Basic/
│   │   └── INA219_Power/
│   └── Modules/
│       ├── Keypad_Input/
│       └── Motor_Drivers/
│
├── 20_Systems_Monitoring/   # [Level 2] 모니터링/제어 "기능" 중심의 완성형 시스템
│   ├── Env_Monitor/         # (환경 모니터링)
│   │   ├── Dust_Bluetooth/  # (구 08_bluetooth_dust_monitoring)
│   │   └── Power_Logger/    # (구 03_P_ina219_powerMonitoring)
│   └── Data_Processing/
│       └── Weigher_Sorter/  # (구 06_weigher-sorter)
│
├── 30_Applications_Real/    # [Level 3] "공간/해결책" 중심의 실전 적용 프로젝트
│   ├── Smart_Home/          # (구 3_가정내_, 4_가정밖_)
│   │   ├── Boiler_Check/
│   │   └── Front_Banner/
│   ├── Smart_Farm/          # (구 09_aquaPonics, smartFarm)
│   │   └── AquaPonics/
│   └── Industrial_Safety/   # (구 11_safety_hat)
│       └── Safety_Hat/
│
├── 99_Archives/             # [종료] 종료되었거나 참조용 레거시 프로젝트
│   ├── Game_Dev/            # (구 6_games, dice_animation)
│   └── Legacy_Proto/        # (구 도로공사차량싸인보드 등)
│
└── README.md                # 전체 저장소 맵(Map) 역할
```

---

## 3. 구조화 상세 설명 & 추상화 전략

### 상위 폴더 1: `10_Components_Basic` (기초 소스)
*   **의도**: "이 센서를 어떻게 쓰지?"에 대한 답을 주는 곳입니다.
*   **특징**: 복잡한 비즈니스 로직, DB 연결, 대시보드 없이 **순수 하드웨어 제어 코드**만 존재합니다.
*   **장점**: 나중에 복잡한 프로젝트를 만들 때 여기서 코드를 `Copy & Paste` 하기 가장 좋습니다.

### 상위 폴더 2: `20_Systems_Monitoring` (기능적 모니터링)
*   **의도**: 사용자님이 말씀하신 **"상황별, 활용성 높은 센서끼리 묶인 모니터링"** 영역입니다.
*   **구성 예시**:
    *   `Dashboard_Integration`: 센서값 + Grafana/Web 연동
    *   `Alert_System`: 값 측정 + 임계치 초과 시 알람
*   **추상화 포인트**: 특정 장소(예: 내 방)에 종속되지 않는 범용적인 시스템입니다.

### 상위 폴더 3: `30_Applications_Real` (도메인 솔루션)
*   **의도**: 기술이 실제 **"문제"**를 해결하는 곳입니다.
*   **특징**: '안방', '베란다', '양어장' 처럼 구체적인 장소나 목적이 명시됩니다. 하드웨어 코드뿐만 아니라 기구 설계(3D모델), 설치 사진 등이 포함될 수 있습니다.

---

## 4. Q&A: 프로젝트 관리에 대한 고민 해결

### Q: 폴더 앞에 번호를 붙여야 하나요? (`01_`, `02_`...)
**A. 대분류에는 번호를 붙이고, 개별 프로젝트에는 번호를 떼는 것을 추천합니다.**
*   **이유**: 개별 프로젝트(예: `01_ds18b20`)에 번호를 매기면, 나중에 `01`과 `02` 사이에 연관된 프로젝트 `01-1`을 넣기가 매우 애매해집니다.
*   **Best Practice**:
    *   최상위 폴더는 정렬을 위해 번호 사용 (`10_`, `20_`, `30_`)
    *   하위 프로젝트는 **이름**으로 관리 (`ds18b20-monitoring`, `smart-farm-v1`)

### Q: 유명한 소규모 프로젝트들은 어떻게 관리하나요?
보통 두 가지 패턴 중 하나를 따릅니다.

1.  **Monorepo (모노레포) 방식** (추천):
    *   지금처럼 하나의 거대한 Git 저장소 안에 여러 패키지(`packages/`)나 앱(`apps/`)을 둡니다.
    *   공통된 라이브러리(유틸리티 함수 등)를 `shared` 폴더에 두고 여러 프로젝트가 가져다 씁니다.

2.  **Domain Driven Design (도메인 주도 설계) 반영**:
    *   기술(`Arduino`, `Python`)로 나누지 않고, 문제 영역(`Farming`, `Security`, `Environment`)으로 폴더를 나눕니다.

### Q: 각 프로젝트 내부 구조(`src`, `docs` 등)는 어떻게?
모든 하위 프로젝트가 **동일한 템플릿**을 가지도록 강제하는 것이 좋습니다.
```text
projectName/
├── docs/          # 기획서, 회로도, 데이터시트
├── src/           # 소스 코드 (main, lib)
├── tests/         # 테스트 코드
├── assets/        # 이미지, 3D 파일
└── README.md      # 필수: 이 프로젝트가 무엇인지 한 줄 요약
```

---

## 5. 단계별 실행 계획 (Action Plan)

1.  **분류 작업**: 위 제안된 4개 대분류(`Components`, `Systems`, `Apps`, `Archives`) 폴더 생성.
2.  **이동 및 정리**: 기존 `01_...` 폴더들을 성격에 맞춰 이동. (이 과정에서 폴더명 변경 권장)
3.  **문서화**: 각 대분류 폴더에 `README.md`를 두어 해당 카테고리의 규칙 명시.
