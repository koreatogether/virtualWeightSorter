# Arduino CLI 사용 가이드

## 1. 보드 목록 확인
```powershell
arduino-cli board list
```

## 2. 컴파일 (Compile)
*   **Arduino R4 WiFi:**
    ```powershell
    arduino-cli compile -b arduino:renesas_uno:unor4wifi <스케치_폴더_경로>
    ```

## 3. 업로드 (Upload)
```powershell
arduino-cli upload -b arduino:renesas_uno:unor4wifi -p COM<번호> <스케치_폴더_경로>
```

## 4. 원라인 빌드 & 업로드 (Build & Upload)
```powershell
arduino-cli compile -b arduino:renesas_uno:unor4wifi <스케치_폴더_경로>; arduino-cli upload -b arduino:renesas_uno:unor4wifi -p COM<번호> <스케치_폴더_경로>
```

> **Note:** 에러 발생 시 터미널 메시지를 확인하고 Copilot에게 "이 에러 해결해줘"라고 요청하세요.
