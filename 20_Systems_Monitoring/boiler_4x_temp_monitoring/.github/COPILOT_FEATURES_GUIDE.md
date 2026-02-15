# GitHub Copilot Features Guide (v2026)

이 문서는 프로젝트에서 활용 가능한 Copilot의 최신 기능(Orchestration, Agents, Custom Skills)에 대한 가이드입니다.

---

## 1. 📂 Copilot Workspace (Orchestration)
단순한 코드 추천을 넘어, **프로젝트 전체 맥락**을 이해하고 다중 파일 작업을 수행하는 기능입니다.

### 주요 기능
-   **Context Awareness**: 현재 열려있는 파일뿐만 아니라, `00_Management`, `LICENSE`, `PROJECT_STRUCTURE_PROPOSAL.md` 등 프로젝트 전체 구조를 인지합니다.
-   **Plan & Execute**: "A 기능을 B 폴더로 옮기고 관련 헤더도 수정해줘"라고 요청하면, Copilot은 다음과 같이 작동합니다.
    1.  관련 파일 검색 (Dependency Graph 분석)
    2.  변경 계획 수립 (A파일 이동, B파일 Import 수정)
    3.  일괄 적용 (Multi-file Editing)

### 활용법 (Project Specific)
이 프로젝트(`e:\project`) 구조 재편 시 다음과 같이 명령할 수 있습니다.
> "legacy 폴더에 있는 DHT22 코드를 `10_Components_Basic`으로 옮기고, 관련된 문서 파일도 찾아서 `docs` 폴더를 합쳐줘."

---

## 2. 🤖 Copilot Agents (Extensions)
특정 도메인 전문 지식을 가진 AI 에이전트를 `@mention`으로 호출하여 사용할 수 있습니다.

### 사용 가능한 Agent 유형
*   **`@workspace`**: 프로젝트 전체 코드를 대상으로 검색하고 질의응답합니다. (가장 많이 사용됨)
*   **`@terminal`**: 터미널 명령어를 추천하거나 실행 결과를 분석해 에러를 수정합니다.
*   **`@vscode`**: VS Code 설정이나 인터페이스 관련 질문을 처리합니다.

---

## 3. 🛠️ Custom Skills (자체 기능 확장)
프로젝트 특화 도구(BOM 체크, 하드웨어 스펙 조회 등)를 Copilot에 연결할 수 있습니다. (MCP - Model Context Protocol 개념)

### 템플릿: 나만의 하드웨어 스펙 조회 에이전트 만들기
프로젝트 루트 `.vscode/mcp-settings.json` (가상 시나리오)에 다음과 같이 등록하여 사용합니다.

```json
/* .vscode/mcp-settings.json 예시 */
{
  "mcpServers": {
    "arduino-helper": {
      "command": "python",
      "args": ["tools/agent_skills.py"],
      "description": "아두이노 센서 스펙 조회 및 BOM 재고 확인 도구"
    }
  }
}
```

### Python 스크립트 예시 (`tools/agent_skills.py`)
이 스크립트를 `tools/` 폴더에 두면 Copilot이 필요할 때 배후에서 실행하여 정보를 가져옵니다.

```python
import sys
import json

# 1. 사용할 도구 정의
def get_sensor_spec(sensor_name):
    specs = {
        "DS18B20": "Voltage: 3.0-5.5V, Pull-up: 4.7kΩ required, Protocol: 1-Wire",
        "DHT22": "Voltage: 3.3-5V, Pins: VCC, DATA, NC, GND",
        "LCD2004": "Interface: I2C (Address 0x27 or 0x3F), Voltage: 5V"
    }
    return specs.get(sensor_name, "Unknown sensor")

# 2. Copilot 요청 처리 (Simulated)
# 실제 환경에서는 MCP SDK를 사용하여 통신합니다.
if __name__ == "__main__":
    # 예: Copilot이 "DS18B20 스펙 알려줘"라고 요청 시 이 함수 호출
    pass
```

---

## 4. 📝 Custom Instructions (지침 파일)
Copilot의 행동 양식을 정의하는 파일로, 이미 `.github/COPILOT_INSTRUCTIONS.md`에 생성되었습니다.
이 파일은 에이전트가 "어떻게 행동해야 하는가"를 정의합니다.

-   **위치**: `.github/COPILOT_INSTRUCTIONS.md`
-   **내용**: 코딩 스타일, 금지 사항(Blocking delay 사용 금지 등), 한글 주석 원칙 등.
