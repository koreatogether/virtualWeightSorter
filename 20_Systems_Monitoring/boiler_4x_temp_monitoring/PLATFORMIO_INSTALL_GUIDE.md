# Google Antigravity - PlatformIO 수동 설치 가이드

이 문서는 Google Antigravity IDE(Windsurf/VSCode 기반)에서 마켓플레이스에 존재하지 않는 PlatformIO를 수동으로 설치하는 과정을 기록합니다.

## 1. 배경
Antigravity는 Open VSX Registry를 사용하는데, PlatformIO 공식 확장이 등록되어 있지 않거나 버전이 낮아 정상 작동하지 않을 수 있습니다. 따라서 GitHub 릴리즈 페이지에서 직접 최신 `.vsix` 파일을 다운로드하여 설치해야 합니다.

## 2. 필수 구성 요소
*   **Antigravity Cli**: 터미널에서 `antigravity --version`으로 확인 가능.
*   **curl.exe** 또는 **Invoke-WebRequest**: VSIX 파일 다운로드용.

## 3. 설치 과정

### 1단계: C/C++ 확장 (ms-vscode.cpptools) 다운로드 및 설치
PlatformIO는 Microsoft의 C/C++ 확장에 의존하므로 이를 먼저 설치해야 합니다.

*   **파일 확인**: [vscode-cpptools GitHub Releases](https://github.com/microsoft/vscode-cpptools/releases)
*   **주의사항**: 파일명이 OS에 따라 다릅니다. Windows x64의 경우 `cpptools-windows-x64.vsix`를 사용합니다. (버전 v1.30.4 기준)
*   **명령어**:
    ```powershell
    curl.exe -L -o cpptools-windows-x64.vsix "https://github.com/microsoft/vscode-cpptools/releases/download/v1.30.4/cpptools-windows-x64.vsix"
    antigravity --install-extension cpptools-windows-x64.vsix
    ```

### 2단계: PlatformIO IDE 확장 다운로드 및 설치
*   **파일 확인**: [platformio-vscode-ide GitHub Releases](https://github.com/platformio/platformio-vscode-ide/releases)
*   **명령어**:
    ```powershell
    curl.exe -L -o platformio-ide.vsix "https://github.com/platformio/platformio-vscode-ide/releases/download/v3.3.4/platformio-ide-3.3.4.vsix"
    antigravity --install-extension platformio-ide.vsix
    ```

### 3단계: 설치 확인 및 정리
*   **설치 확인**:
    ```powershell
    antigravity --list-extensions
    ```
    결과 리스트에 `ms-vscode.cpptools`와 `platformio.platformio-ide`가 있으면 성공입니다.
*   **임시 파일 삭제**:
    ```powershell
    del cpptools-windows-x64.vsix, platformio-ide.vsix
    ```

## 4. 트러블슈팅 (Troubleshooting)
1.  **다운로드 실패 (9 bytes 파일 생성)**: GitHub 다운로드 링크가 Direct Download 링크가 아닐 경우 발생합니다. `curl -L` 옵션을 사용하거나, HEAD 요청(`curl -I`)을 통해 실제 Location URL을 확인해야 합니다.
2.  **명령어 오류**: PowerShell에서 `del` 명령어 사용 시 여러 파일을 삭제하려면 반드시 쉼표(`,`)로 구분해야 합니다.
    *   `del file1 file2` (X) -> `del file1, file2` (O)
3.  **의존성 오류**: 반드시 `cpptools`를 먼저 설치해야 PlatformIO가 안정적으로 작동합니다.

## 5. 참고 자료
*   가이드 출처: [Lynx's Blog](https://www.einherjar.org/2026/01/19/installing-platformio-in-google-antigravity/)
*   Antigravity 공식: [antigravity.google](https://antigravity.google/)
