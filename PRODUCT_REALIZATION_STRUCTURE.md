# 실증·제품 개발 중심 폴더 구조 제안 (아이디어 확장 → 프로젝트 분리 관점)

## 결론
아이디어를 넓게 확장하다가 **어느 시점에 모아 프로젝트로 분리**하는 흐름을 기준으로 구조를 재정리했습니다.  
핵심은 다음 두 단계입니다.
1. **아이디어 확장/수집 단계**: 가볍게 쌓아두는 공간
2. **프로젝트 분리 단계**: 명확한 목표와 산출물을 갖는 개발형 폴더 구조로 전환

따라서 아래처럼 **아이디어 풀**과 **프로젝트 개발 구역**을 분리하는 구조를 권장합니다.

## 추천 루트 구조 (아이디어 풀 → 프로젝트 전환 포함)
```text
E:\project\
├─ 00_Management/                 # 아이디어, 전략, 예산, 일정, 정책
├─ 05_Idea_Pool/                   # 확장/수집 단계 아이디어 보관
├─ 10_Components_Basic/           # 부품 단위 실험/PoC
├─ 20_Systems_Monitoring/         # 시스템 단위 기능 구현/검증
├─ 35_Projects/                    # 프로젝트로 분리된 개발형 폴더 구조
├─ 40_Pilots_Validation/          # 실증/파일럿/현장 검증
├─ 50_Product_Development/        # 제품 개발 (요구사항→설계→시험→양산 준비)
├─ 60_Operations_Support/         # 출시 후 운영, 유지보수, 고객지원
├─ 90_References/                 # 자료/레퍼런스
└─ 99_Archives/                   # 종료/보관
```

## 운영 원칙 (프로젝트 수준에서 종료)
- 최종 목표는 **프로젝트 수준에서 완료**하는 것으로 설정합니다.
- 따라서 `30_Applications_Real` 같은 응용/서비스 단계는 운영하지 않습니다.
- 프로젝트 결과물은 `35_Projects` 내부에서 완결되고, 필요 시 실증/제품화 영역으로만 확장합니다.

## 05_Idea_Pool (아이디어 확장/수집)
```text
05_Idea_Pool/
├─ Raw/                            # 거친 발상, 짧은 메모
├─ Notes/                          # 정리된 아이디어
├─ Clusters/                       # 유사 아이디어 묶음
└─ Seeds_For_Project/              # 프로젝트 후보군
```

## 35_Projects (프로젝트 분리 후 개발형 구조)
```text
35_Projects/
├─ Project_Alpha/
│  ├─ 00_Requirements/             # 요구사항/목표/KPI
│  ├─ 10_System_Design/             # 아키텍처/인터페이스
│  ├─ 20_Hardware/                  # 회로/PCB/BOM/기구
│  ├─ 30_Firmware/                  # 펌웨어/OTA
│  ├─ 40_Software/                  # 앱/서버/대시보드
│  ├─ 50_Test_Verification/         # 테스트 계획/결과
│  ├─ 60_Certifications/            # 인증 문서
│  ├─ 70_Manufacturing/             # 제조/검사
│  └─ 80_Release/                   # 릴리즈 패키지
└─ Project_Beta/
   └─ (동일 구조)
```

## 40_Pilots_Validation (실증/파일럿)
```text
40_Pilots_Validation/
├─ Sites/                         # 실증 사이트별 폴더
│  ├─ 2026-01_서울A공장/
│  └─ 2026-02_부산B체육센터/
├─ Pilot_Plans/                   # 실증 계획서, 목표, KPI, 리스크
├─ Field_Logs/                    # 현장 로그, 점검표, 장애 기록
├─ Data_Reports/                  # 데이터 결과, 성능 보고서
└─ Lessons_Learned/               # 회고, 개선사항
```

## 50_Product_Development (제품 개발)
```text
50_Product_Development/
├─ 00_Requirements/               # 요구사항, 사용자 스토리, 규격
├─ 10_System_Design/              # 시스템 아키텍처, 인터페이스 정의
├─ 20_Hardware/                   # 회로, PCB, BOM, 기구/3D
├─ 30_Firmware/                   # 펌웨어, 부트로더, OTA
├─ 40_Software/                   # 앱/서버/대시보드
├─ 50_Test_Verification/          # 시험 계획, 테스트 결과
├─ 60_Certifications/             # KC/CE/FCC 등 인증 자료
├─ 70_Manufacturing/              # 제조 공정, 지그, 검사 기준
└─ 80_Release/                    # 출시 패키지, 릴리즈 노트
```

## 60_Operations_Support (운영/지원)
```text
60_Operations_Support/
├─ Monitoring/                    # 모니터링/알림 규칙
├─ Maintenance/                   # 유지보수 매뉴얼
├─ Support_FAQ/                   # 고객지원 FAQ
└─ RMA_Returns/                   # 불량/반품 기록
```

## 적용 가이드
1. 아이디어는 먼저 `05_Idea_Pool`에 쌓습니다.
2. 후보가 모이면 `Seeds_For_Project`로 이동해 프로젝트화 여부를 판단합니다.
3. 프로젝트로 확정되면 `35_Projects/프로젝트명/`으로 분리하고 개발형 구조를 적용합니다.
4. 실증 자료는 `40_Pilots_Validation`으로, 제품화 산출물은 `50_Product_Development`로 이동합니다.
5. 운영 자료는 `60_Operations_Support`에 별도 관리합니다.

## 체크 포인트 (실증·제품 개발 관점)
- 실증 계획서와 결과 보고서가 동일 위치에 모이는가
- 인증/양산 준비 문서가 코드와 분리되어 있는가
- 운영·유지보수 자료가 출시 전 단계와 분리되어 있는가
