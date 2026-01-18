# Boiler Temp Monitoring 📡

간단 소개:
Boiler 및 분배기(매니폴드)의 온도 모니터링 시스템 템플릿입니다. 이 디렉터리는 프로젝트 구조 제안서(`PROJECT_STRUCTURE_PROPOSAL.md`)에 따라 표준화된 하위 폴더(`docs`, `src`, `tests`, `dashboard`, `alert_rules` 등)를 포함하며, 초기 개발·설계에 필요한 기본 파일을 제공합니다.

구성(초기):
- `docs/` : 아키텍처·설계 문서
- `src/` : 소스 코드 (데이터 수집·로직)
- `tests/` : 단위/통합 테스트
- `dashboard/` : Grafana/대시보드 관련 구성 및 예시
- `alert_rules/` : 알람 룰(예: Prometheus Alertmanager/YAML)
- `configs/` : 예시 설정 파일

빠른 시작:
1. `src/main.py`를 참고해 센서 드라이버 또는 시뮬레이터를 실행하세요.
2. `docs/ARCHITECTURE.md`를 확인해 시스템 경계와 통신 인터페이스를 검토하세요.

---

(자동 생성된 초기 템플릿 파일들입니다. 필요하시면 BOM·구매 링크·샘플 데이터까지 채워드리겠습니다.)