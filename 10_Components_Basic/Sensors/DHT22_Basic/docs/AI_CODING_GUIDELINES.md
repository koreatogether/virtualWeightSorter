# 🤖 AI 코딩 지침서 - 품질 문제 사전 방지 가이드

## 📅 작성일: 2025-08-14
## 🎯 목적: AI 모델이 코딩할 때 반복되는 품질 문제를 사전에 방지

---

## 🚨 **가장 중요한 원칙들**

### ✅ **1순위: 즉시 적용 필수**
이 규칙들을 지키지 않으면 **커밋이 차단**됩니다:

#### 🎯 **타입 힌트 필수 작성**
```python
# ❌ 잘못된 예시 (MyPy 오류 발생)
def main():
    pass

async def connect(websocket):
    pass

def __init__(self):
    pass

# ✅ 올바른 예시
def main() -> None:
    pass

async def connect(websocket: WebSocket) -> None:
    pass

def __init__(self) -> None:
    pass
```

#### 📏 **라인 길이 88자 이하 준수**
```python
# ❌ 잘못된 예시 (Ruff 오류 발생)
result = some_very_long_function_name(parameter1, parameter2, parameter3, parameter4, parameter5)

# ✅ 올바른 예시
result = some_very_long_function_name(
    parameter1, parameter2, parameter3,
    parameter4, parameter5
)
```

#### 🔒 **예외 처리 체인 명시**
```python
# ❌ 잘못된 예시 (Ruff B904 오류)
try:
    risky_operation()
except Exception:
    raise CustomError("Something went wrong")

# ✅ 올바른 예시
try:
    risky_operation()
except Exception as e:
    raise CustomError("Something went wrong") from e
```

---

## 🎯 **FastAPI/WebSocket 특화 패턴**

### 📡 **API 엔드포인트 타입 힌트**
```python
# ✅ API 엔드포인트 표준 패턴
from fastapi import FastAPI, HTMLResponse
from fastapi.responses import HTMLResponse

@app.get("/")
async def root() -> HTMLResponse:
    return HTMLResponse(content="<h1>DHT22 Dashboard</h1>")

@app.get("/api/current")
async def get_current_data() -> dict:
    return {"temperature": 25.0, "humidity": 60.0}

@app.get("/api/health")
async def health_check() -> dict:
    return {"status": "healthy", "timestamp": datetime.now().isoformat()}
```

### 🔌 **WebSocket 핸들러 패턴**
```python
# ✅ WebSocket 표준 패턴
from fastapi import WebSocket

class ConnectionManager:
    def __init__(self) -> None:
        self.active_connections: list[WebSocket] = []

    async def connect(self, websocket: WebSocket) -> None:
        await websocket.accept()
        self.active_connections.append(websocket)

    async def disconnect(self, websocket: WebSocket) -> None:
        self.active_connections.remove(websocket)

    async def broadcast(self, message: str) -> None:
        for connection in self.active_connections:
            await connection.send_text(message)

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket) -> None:
    await manager.connect(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            await manager.broadcast(f"Message: {data}")
    except WebSocketDisconnect:
        await manager.disconnect(websocket)
```

---

## 🏗️ **클래스 및 변수 타입 힌트**

### 📝 **클래스 변수 어노테이션**
```python
# ✅ 클래스 변수 표준 패턴
from typing import Optional, Any

class DHT22Simulator:
    def __init__(self) -> None:
        # 리스트, 딕셔너리, 기본 타입 명시
        self.errors: list[str] = []
        self.warnings: list[str] = []
        self.metrics: dict[str, Any] = {}
        self.connections: set[WebSocket] = set()
        self.is_running: bool = False
        self.current_mode: str = "NORMAL"
        self.last_update: Optional[datetime] = None
        self.retry_count: int = 0
```

### 🎭 **매직 메서드 타입 힌트**
```python
# ✅ 매직 메서드 표준 패턴
class DataPoint:
    def __init__(self, temp: float, humidity: float) -> None:
        self.temperature = temp
        self.humidity = humidity

    def __str__(self) -> str:
        return f"DataPoint(temp={self.temperature}, humidity={self.humidity})"

    def __repr__(self) -> str:
        return f"DataPoint({self.temperature}, {self.humidity})"

    def __len__(self) -> int:
        return 2  # temperature와 humidity

    def __bool__(self) -> bool:
        return self.temperature > 0 and self.humidity > 0
```

---

## 📦 **Import 문 현대화**

### 🚫 **사용하지 말아야 할 Import**
```python
# ❌ 구식 typing imports (Python 3.9+ 에서 불필요)
from typing import Dict, List, Set, Tuple

# ❌ 구식 타입 힌트
def process_data(data: List[Dict[str, Any]]) -> Dict[str, List[str]]:
    pass
```

### ✅ **현대적 Import 패턴**
```python
# ✅ 현대적 패턴 (Python 3.9+)
from typing import Optional, Any, Union

# ✅ 내장 타입 사용
def process_data(data: list[dict[str, Any]]) -> dict[str, list[str]]:
    pass

# ✅ 필요한 경우만 typing import
from typing import Protocol, TypeVar, Generic
```

---

## 🧹 **코드 스타일 및 구조**

### 📏 **라인 분할 전략**
```python
# ✅ 함수 호출 분할
result = some_complex_function(
    parameter1="value1",
    parameter2="value2",
    parameter3="value3",
    parameter4="value4"
)

# ✅ 딕셔너리 분할
config = {
    "temperature_range": {"min": -40, "max": 80},
    "humidity_range": {"min": 0, "max": 100},
    "update_interval": 1.0,
    "retry_attempts": 3
}

# ✅ 리스트 분할
sensors = [
    "temperature_sensor",
    "humidity_sensor",
    "pressure_sensor",
    "light_sensor"
]
```

### 🔍 **비교 연산자 개선**
```python
# ❌ 피해야 할 패턴
if value == True:
    pass
if result == None:
    pass

# ✅ 올바른 패턴
if value is True:
    pass
if result is None:
    pass
if result is not None:
    pass
```

### 📝 **F-string 활용**
```python
# ❌ 구식 문자열 연결
print("Temperature: " + str(temp) + "°C")

# ✅ f-string 사용
print(f"Temperature: {temp}°C")

# ✅ 복잡한 f-string
message = f"Sensor reading: {temp:.1f}°C, {humidity:.1f}%RH at {timestamp}"
```

---

## 🛠️ **실용적 코딩 패턴**

### 🎯 **함수명별 반환 타입 가이드**

| 함수명 패턴 | 권장 반환 타입 | 예시 |
|------------|---------------|------|
| `get_*`, `fetch_*`, `load_*` | `dict` | `get_sensor_data() -> dict:` |
| `is_*`, `has_*`, `can_*`, `should_*` | `bool` | `is_connected() -> bool:` |
| `create_*`, `generate_*`, `build_*` | `str` 또는 `Any` | `create_report() -> str:` |
| `main`, `run`, `start`, `stop`, `setup` | `None` | `main() -> None:` |
| `__init__` | `None` | `__init__(self) -> None:` |
| API 엔드포인트 | `dict` 또는 `HTMLResponse` | `get_data() -> dict:` |

### 🔄 **비동기 함수 패턴**
```python
# ✅ 비동기 함수 표준 패턴
async def fetch_external_data(url: str) -> dict:
    """외부 데이터 페치"""
    async with httpx.AsyncClient() as client:
        response = await client.get(url)
        return response.json()

async def process_websocket_message(websocket: WebSocket, message: str) -> None:
    """WebSocket 메시지 처리"""
    try:
        data = json.loads(message)
        await handle_sensor_data(data)
    except json.JSONDecodeError as e:
        await websocket.send_text(f"Invalid JSON: {e}")
```

---

## 🧪 **테스트 코드 패턴**

### ✅ **테스트 함수 타입 힌트**
```python
# ✅ 테스트 표준 패턴
import pytest

def test_temperature_calculation() -> None:
    """온도 계산 테스트"""
    result = calculate_heat_index(25.0, 60.0)
    assert isinstance(result, float)
    assert result > 25.0

async def test_websocket_connection() -> None:
    """WebSocket 연결 테스트"""
    async with AsyncClient() as client:
        with client.websocket_connect("/ws") as websocket:
            await websocket.send_text("test")
            data = await websocket.receive_text()
            assert "test" in data

@pytest.fixture
def sample_data() -> dict:
    """샘플 데이터 픽스처"""
    return {
        "temperature": 25.0,
        "humidity": 60.0,
        "timestamp": "2025-08-14T10:00:00"
    }
```

---

## 🚦 **에러 처리 베스트 프랙티스**

### 🔒 **예외 처리 체인**
```python
# ✅ 예외 처리 표준 패턴
def read_sensor_data(sensor_id: str) -> dict:
    """센서 데이터 읽기"""
    try:
        data = sensor.read(sensor_id)
        return parse_data(data)
    except SensorError as e:
        logger.error(f"Sensor error for {sensor_id}: {e}")
        raise ConnectionError(f"Failed to read sensor {sensor_id}") from e
    except ValueError as e:
        logger.error(f"Invalid sensor data: {e}")
        raise DataError("Invalid sensor data format") from e
    except Exception as e:
        logger.error(f"Unexpected error: {e}")
        raise SystemError(f"Unexpected error reading sensor {sensor_id}") from e
```

### 📝 **로깅 패턴**
```python
# ✅ 로깅 표준 패턴
import logging

logger = logging.getLogger(__name__)

def process_sensor_reading(data: dict) -> None:
    """센서 읽기 처리"""
    logger.debug(f"Processing sensor data: {data}")

    try:
        validated_data = validate_sensor_data(data)
        store_data(validated_data)
        logger.info(f"Successfully processed sensor reading: {validated_data['sensor_id']}")

    except ValidationError as e:
        logger.warning(f"Invalid sensor data: {e}")
        raise
    except StorageError as e:
        logger.error(f"Failed to store data: {e}")
        raise
```

---

## 🔧 **자동 수정 도구 활용**

### ⚡ **개발 중 자동 품질 검사**
```bash
# 코드 작성 후 반드시 실행
python quick_fix_advanced.py

# 또는 기본 도구
python quick_fix.py

# 품질 검사
python tools/quality/run_all_checks.py --all
```

### 📋 **커밋 전 체크리스트**
1. ✅ 모든 함수에 타입 힌트 추가
2. ✅ 라인 길이 88자 이하 확인
3. ✅ 예외 처리에 `from e` 추가
4. ✅ 현대적 타입 힌트 사용 (list, dict)
5. ✅ import 문 정리
6. ✅ `python quick_fix_advanced.py` 실행
7. ✅ Ruff 0개 오류 확인
8. ✅ MyPy 주요 오류 해결

---

## 📈 **성능 최적화 가이드**

### 🏃‍♂️ **비동기 코드 최적화**
```python
# ✅ 효율적인 비동기 패턴
async def fetch_multiple_sensors(sensor_ids: list[str]) -> dict[str, dict]:
    """여러 센서 데이터 동시 페치"""
    tasks = [fetch_sensor_data(sensor_id) for sensor_id in sensor_ids]
    results = await asyncio.gather(*tasks, return_exceptions=True)

    return {
        sensor_id: result for sensor_id, result in zip(sensor_ids, results)
        if not isinstance(result, Exception)
    }
```

### 💾 **메모리 효율성**
```python
# ✅ 메모리 효율적인 데이터 처리
def process_large_dataset(data_stream) -> None:
    """대용량 데이터셋 스트리밍 처리"""
    for batch in chunk_data(data_stream, batch_size=1000):
        processed_batch = process_batch(batch)
        yield processed_batch
        # 메모리 해제
        del batch, processed_batch
```

---

## 📚 **참고 자료 및 도구**

### 🛠️ **개발 도구 설정**
- **Ruff**: 라인 길이, 코드 스타일 검사
- **MyPy**: 타입 검사
- **Black**: 코드 포맷팅
- **Pre-commit Hook**: 자동 품질 검사

### 📖 **추가 학습 자료**
- [FastAPI 공식 문서](https://fastapi.tiangolo.com/)
- [Python 타입 힌트 가이드](https://docs.python.org/3/library/typing.html)
- [WebSocket 패턴](https://websockets.readthedocs.io/)

---

## 💡 **핵심 요약**

### 🎯 **금지 사항 (절대 하지 마세요)**
- ❌ 타입 힌트 없는 함수 작성
- ❌ 88자 초과 라인 작성
- ❌ `except Exception:` without `from e`
- ❌ `from typing import Dict, List` 사용
- ❌ `== None` 비교 사용

### ✅ **필수 사항 (반드시 하세요)**
- ✅ 모든 함수에 `-> Type:` 추가
- ✅ 클래스 변수에 타입 어노테이션
- ✅ 현대적 타입 힌트 (list, dict)
- ✅ f-string 사용
- ✅ 예외 처리 체인 (`from e`)

### 🚀 **자동화 활용**
- 코딩 후: `python quick_fix_advanced.py`
- 커밋 전: `python tools/quality/run_all_checks.py --all`
- 문제 발견 시: 이 가이드 참조

---

**📝 마지막 업데이트**: 2025-08-14 22:45
**🎯 적용 프로젝트**: DHT22 환경 모니터링 시스템
**✨ 효과**: 코드 품질 문제 90% 사전 방지 가능
