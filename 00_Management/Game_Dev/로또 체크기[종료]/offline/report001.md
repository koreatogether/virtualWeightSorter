
// filepath: report001.md

# 코드 리뷰 보고서

## 개요
- Lotto_checker_003.ino와 lotto_checker_004.ino 파일 비교
- 동일 기능이 함수로 분리되어 가독성 향상
- 보너스 번호 입력 처리 로직이 명확해짐

## 상세 리뷰
- 003 파일에 있던 중복 검사, 유효성 체크 등을 함수로 옮겨 재활용
- readWinningNumbers, readBonusNumber, readUserNumbersAndCheck 등으로 역할 분리
- 입력 데이터 유효하지 않은 경우 메시지 출력 후 함수 종료 처리
- 가독성 및 유지보수성 개선

## 개선 제안
- 전역 변수 사용 최소화
- 테스트 자동화 코드 도입 고려
- 함수별 에러 처리 로직 보강