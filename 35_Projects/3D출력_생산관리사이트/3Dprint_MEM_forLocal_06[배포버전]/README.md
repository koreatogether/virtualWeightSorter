# 3D Printing Production Management System (MES Lite) - Local v04

**3D 출력 생산 관리 시스템**은 소규모 3D 프린팅 공방이나 개인 제작자를 위한 로컬 기반 생산 관리 도구입니다.  
주문(Order), 생산 일정(Schedule), 필라멘트 재고(Inventory), 프린터 자산(Asset)을 통합 관리하여 효율적인 생산 흐름을 지원합니다.

---

## 🚀 주요 기능 (Key Features)

### 1. 주문 관리 (Order Management)
- **생산 목표 추적**: 목표 수량, 기확보 재고, 생산 진행률 실시간 시각화.
- **사진 관리**: 제품 사진 업로드 및 미리보기 지원.
- **스마트 입력**: 최근 생산 품목 퀵 필(Quick Fill) 및 기존 무게 데이터 추천.
- **상태 관리**: 진행 중(In Progress) / 완료(Completed) 자동 전환.

### 2. 생산 일정 관리 (Production Scheduling)
- **자동 스케줄링**: 일일 생산 능력(Capa) 기반으로 장기 생산 계획 자동 생성.
- **타임라인 뷰 (Gantt Chart)**: 프린터별 가동 일정을 7일간 시각적으로 확인.
- **중복 감지**: 동일 프린터/시간대 중복 예약 시 경고 알림.
- **실적 기반 연동**: 실적 입력 시 미래 계획 자동 차감 및 초과 생산분 반영.

### 3. 필라멘트 재고 관리 (Inventory Management)
- **정밀 무게 추적**: 개별 스풀 단위(g) 잔량 관리 및 소수점 지원.
- **배치(Batch) 시스템**: A1~A4 등 배치 번호를 지정하여 로트별 관리 가능.
- **예상 소모량 산출**:
    - **실시간 계산**: 등록된 생산 일정을 기반으로 현재 남은 **'추정 잔량'** 자동 계산.
    - **상태 알림**: [충분/부족] 상태를 즉시 판별하여 색상(Green/Red)으로 표시.
    - **통합 뷰**: 동일 재질/색상의 여러 배치를 합산하여 부족 여부 판단.

### 4. 프린터 자산 관리 (Printer Asset Management)
- **장비 이력**: 구입일, 모델명, 고유 자산 ID 관리.
- **유지보수 로그**: 노즐 교체, 레벨링 등 정비 이력 기록 및 최근 점검일 표시.
- **가동률 분석**: 프린터별 총 작업 건수 및 생산량 통계 제공.

### 5. 통계 및 리포트 (Statistics & Reporting)
- **재료 소모량 집계**: 재질/색상별 총 사용량(g) 분석.
- **생산 랭킹**: 상위 5개 다생산 품목 리스트 제공.
- **대시보드**: 전체 공정 진행률 및 가동 중인 프린터 현황 실시간 모니터링.

---

## 🛠 기술 스택 (Tech Stack)
- **Backend**: Python (Flask)
- **Database**: JSON File System (NoSQL-like structure suitable for local use)
- **Frontend**: Vanilla JavaScript, HTML5, CSS3 (No build step required)
- **Deployment**: Local Server (Development server)

---

## 📦 설치 및 실행 (Installation & Usage)

### 전제 조건 (Prerequisites)
- Python 3.8 이상 설치 필요

### 실행 방법 (Windows)
1. 프로젝트 폴더 내 `RUN_LOCAL.ps1` 파일을 우클릭하여 **PowerShell에서 실행**하세요.
   (또는 터미널에서 `.\RUN_LOCAL.ps1` 입력)
2. 자동으로 가상환경(`.venv`)이 생성되고 필요한 패키지가 설치됩니다.
3. 서버가 시작되면 브라우저가 열리며 `http://127.0.0.1:5000`으로 접속됩니다.

---

## 📂 폴더 구조 (Directory Structure)

```
3Dprint_MEM_forLocal_04/
├── 40_Software/
│   ├── app.py              # 메인 백엔드 애플리케이션
│   ├── requirements.txt    # 의존성 패키지 목록
│   ├── static/
│   │   ├── css/            # 스타일시트
│   │   ├── js/             # 프론트엔드 로직 (main.js)
│   │   └── uploads/        # 업로드된 이미지 저장소
│   ├── templates/          # HTML 템플릿
│   └── data/
│       ├── db.json         # 메인 데이터베이스
│       └── user_actions.log # 감사 로그 (Audit Log)
├── docs/                   # 프로젝트 문서
├── progressLog.md          # 개발 진행 일지
└── RUN_LOCAL.ps1           # 원클릭 실행 스크립트
```

---

## 📝 라이선스 및 크레딧
- **개발**: Google Gemini (Antigravity Agent) & User Collaboration
- **버전**: 04 (2026-02-12)
