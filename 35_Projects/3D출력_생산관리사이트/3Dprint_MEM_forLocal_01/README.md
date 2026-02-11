# 3D 출력 생산 관리 사이트

3D 출력물의 생산 일정 관리 및 수량 파악을 위한 로컬 웹사이트 프로젝트입니다.

## 프로젝트 정보
- **상태:** Requirements Analysis (요구사항 분석 중)
- **시작일:** 2026-02-11
- **목표:** 효율적인 3D 프린팅 생산 공정 관리 시스템 구축

## 폴더 구조
- `00_Requirements`: 요구사항 정의 및 분석 문서
- `10_System_Design`: 시스템 아키텍처 및 DB 설계
- `20_Hardware`: (해당 시) 프린터 하드웨어 설정 관련
- `30_Firmware`: (해당 시) 프린터 펌웨어 관련
- `40_Software`: 웹사이트 소스 코드 (Python Flask)
- `50_Test_Verification`: 테스트 계획 및 결과
- `docs`: 일반 문서 및 설문지

## 진행 상황
- [2026-02-11] 프로젝트 폴더 구조 생성 및 요구사항 질문지 작성 완료
- [2026-02-11] 시스템 설계 (Data Schema, Architecture) 완료
- [2026-02-11] Flask 기반 기본 웹 서버 및 AI-Friendly HTML 구조 구축
- [2026-02-11] 로컬 가상환경(.venv) 설정 및 의존성 설치 완료

## 시작하기 (How to Run)
1. Python 설치 확인
2. `40_Software` 폴더로 이동
3. 가상환경 활성화 (Windows):
   ```powershell
   .\.venv\Scripts\activate
   ```
4. 서버 실행:
   ```powershell
   python app.py
   ```
5. 웹 브라우저에서 `http://127.0.0.1:5000` 접속