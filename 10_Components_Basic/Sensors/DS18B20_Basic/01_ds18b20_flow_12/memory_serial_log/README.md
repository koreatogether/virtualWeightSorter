# memory_serial_log

이 폴더는 Arduino UNO R4 WiFi 보드의 실시간 메모리 사용량 로그를 시리얼로 받아서 PC에 저장하기 위한 목적입니다.

## 구성 파일
- `save_serial_log.py`: 시리얼(COM4)로 출력되는 메모리 로그를 `memory_log.txt`로 저장하는 파이썬 스크립트
- `memory_log.txt`: 파이썬 스크립트 실행 시 자동 생성되는 실시간 메모리 로그 파일

## 사용 방법
1. 아두이노 보드가 COM4로 연결되어 있고, 메모리 로그를 시리얼로 출력하도록 코드가 작성되어 있어야 합니다.
2. PC에서 `save_serial_log.py`를 실행하면, 시리얼 로그가 `memory_log.txt`로 저장됩니다.
   ```bash
   python save_serial_log.py
   ```
3. 로그 파일은 실시간으로 갱신되며, 분석 및 보고서 작성에 활용할 수 있습니다.

## 참고
- 로그 파일은 스크립트 실행 디렉토리에 생성됩니다.
- 필요에 따라 파이썬 코드의 포트/보드레이트를 수정하세요.
