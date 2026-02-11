# System Architecture

## 1. 개요
본 시스템은 3D 출력 생산 관리를 위한 로컬 웹 애플리케이션으로, 인간 사용자와 AI 에이전트 모두에게 최적화된 인터페이스를 제공하는 것을 목표로 합니다.

## 2. 기술 스택
- **Backend:** Python (Flask) - 경량화 및 빠른 API 구축
- **Frontend:** HTML5, Vanilla JS (또는 최소한의 UI 라이브러리), CSS (Simple/Dark mode)
- **Database:** JSON / JSONL (로컬 파일 기반 저장)
- **Communication:** RESTful API

## 3. 구성 요소
- **Web Server:** API 요청 처리 및 정적 파일(HTML/JS/CSS) 서빙
- **Data Layer:** JSON 파일을 읽고 쓰는 File I/O 관리자
- **UI Layer:** 
    - Dashboard: 생산 현황 시각화
    - Calendar/Gantt: 일정 관리
    - Management: 필라멘트 및 출력물 로그 관리

## 4. AI 에이전트 친화적 설계
- **Semantic HTML:** 시각적 화려함보다는 데이터 구조를 반영하는 HTML 태그 사용.
- **REST API:** 모든 기능은 API로 접근 가능하여 에이전트가 직접 데이터를 수정하거나 조회할 수 있음.
- **Documentation:** API 엔드포인트에 대한 명확한 설명을 제공하여 에이전트의 오작동 방지.
