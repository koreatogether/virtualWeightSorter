**WORK IN PROGRESS — Rev03 (stabilization branch, copied from Rev02)**

`NOTE`: This directory was created from `mainRev08_seeed_esp32C3_02` to perform field testing, bugfixing and integration. See `raptor_checklist.md` / `raptor_checklist.json` in this folder for tests and required inputs.

eps32 c3 의 데이타 시트 파일 저장위치 : 
E:\project\20_Systems_Monitoring\boiler_4x_temp_monitoring\docs\ESP32_C3\esp32-c3_datasheet.pdf

사용보드 : seeed XIAO ESP32-C3

5V (VBUS / USB) 핀은 보드의 전원 입력/출력으로 사용됩니다. 사용 가능한 전류는 USB 호스트/포트와 보드의 전원 회로(레귤레이터, 퓨즈 등)에 따라 다르며, 일반 USB2.0 표준 포트는 최대 500mA를 제공할 수 있으나 포트나 보드에 따라 더 낮을 수 있습니다. 주변기기의 전력 요구가 큰 경우 별도 외부 전원을 사용하는 것을 권장합니다.

3.3V 전원 공급핀은 보드의 3.3V 레귤레이터에서 공급되는 전원을 출력합니다. 이 핀에서 사용할 수 있는 최대 전류는 보드에 사용된 레귤레이터의 사양에 따라 다르며, 일반적으로 수백 밀리암페어(예: 500mA ~ 1A) 수준입니다. 주변기기의 전력 요구가 높을 경우 레귤레이터의 최대 용량을 초과하지 않도록 주의해야 합니다.

3.3V (GPIO) 출력은 핀 당 보수적으로 약 12mA 수준을 권장합니다. 데이터시트의 절대 최대값은 더 높을 수 있으므로, 다수의 핀에서 큰 전류를 동시에 사용할 경우 전체 3.3V 레일 용량과 발열을 고려하세요.
