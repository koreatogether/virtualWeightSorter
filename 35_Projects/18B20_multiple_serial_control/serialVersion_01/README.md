# 18B20_multiple_serial_control

간단 설명:
- 여러 DS18B20 센서를 시리얼 콘솔로 제어·모니터링하는 예제 프로젝트입니다.
- `.github`의 코딩 규칙·빌드 가이드를 준수합니다(특히 DallasTemperature, OneWire 권장).

구성 가이드:
- 본 폴더는 프로젝트 템플릿(`00_Requirements` ~ `80_Release`)을 따릅니다.
- 먼저 하드웨어 연결(핀, 풀업 저항)과 펌웨어(예제 스케치)를 준비하세요.

다음 작업(권장):
1. `30_Firmware/01_multiple_ds18b20_serial`에 예제 스케치 추가
2. `20_Hardware`에 회로도·핀맵 문서 추가
3. `00_Requirements`에 요구사항(목표, 보드, 부품) 정리

문서/코드 작성 시 `.github/COPILOT_INSTRUCTIONS.md`의 코딩 규칙을 따르세요.