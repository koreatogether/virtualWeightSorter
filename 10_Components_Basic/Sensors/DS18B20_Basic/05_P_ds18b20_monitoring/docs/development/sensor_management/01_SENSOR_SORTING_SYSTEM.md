# DS18B20 센서 정렬 시스템 설계

**작성일**: 2025-08-23  
**목적**: 다중 DS18B20 센서의 우선순위 기반 정렬 및 표시 시스템  
**상태**: 구현 완료

---

## 🎯 설계 목표

### 정렬 요구사항
DS18B20 센서는 EEPROM 설정 상태에 따라 다음과 같이 분류되고 정렬되어야 함:

1. **1순위**: EEPROM 설정된 센서 (sensor_id: 01-08)
   - sensor_id 숫자 순서로 정렬 (01, 02, 03, ...)
2. **2순위**: EEPROM 미설정 센서 (sensor_id: 00)
   - 16자리 고유주소 문자열 알파벳 순서로 정렬
3. **3순위**: 오류/기타 센서
   - 고유주소 문자열 순서로 정렬

### 표시 요구사항
- 대시보드에 최대 8개 센서만 표시
- 연결된 센서만 동적으로 표시
- 연결 해제된 센서는 자동으로 목록에서 제거
- 실시간 센서 데이터 업데이트

---

## 🏗️ 아키텍처 설계

### 1. SensorData 데이터 클래스

```python
@dataclass
class SensorData:
    """단일 센서 데이터"""
    temperature: float          # 온도 값 (°C)
    sensor_addr: str           # 16자리 고유주소 (예: 285882840000000E)
    sensor_id: str             # EEPROM 센서 ID (00-08)
    user_sensor_id: int        # 사용자 정의 ID
    th_value: int              # 고온 임계값
    tl_value: int              # 저온 임계값
    measurement_interval: int   # 측정 간격 (ms)
    timestamp: int             # 타임스탬프
    priority_group: int        # 우선순위 그룹 (1=설정됨, 2=미설정, 3=오류)
```

### 2. SensorDataManager 클래스

```python
class SensorDataManager:
    """센서 데이터 수집, 정렬, 관리"""
    
    def __init__(self):
        self._sensors: Dict[str, SensorData] = {}  # key: sensor_addr
        self._last_update_time = 0
    
    # 핵심 메서드
    def add_sensor_data(self, json_data: dict) -> None
    def get_sorted_sensors(self) -> List[SensorData]
    def get_sensors_by_group(self) -> Dict[int, List[SensorData]]
    def get_display_info(self, max_sensors: int = 8) -> List[Dict]
    def clear_old_sensors(self, max_age_seconds: int = 10) -> int
```

---

## 🔄 정렬 로직 구현

### 정렬 키 함수

```python
def sort_key(sensor: SensorData) -> tuple:
    """센서 정렬을 위한 키 함수"""
    if sensor.priority_group == 1:
        # 1순위: sensor_id로 숫자 정렬 (01, 02, 03, ...)
        return (sensor.priority_group, int(sensor.sensor_id))
    else:
        # 2순위, 3순위: 고유주소로 문자열 사전순 정렬
        return (sensor.priority_group, sensor.sensor_addr)
```

### 우선순위 그룹 결정

```python
def determine_priority_group(sensor_id: str) -> int:
    """센서 ID를 기반으로 우선순위 그룹 결정"""
    if sensor_id in ['01', '02', '03', '04', '05', '06', '07', '08']:
        return 1  # EEPROM 설정된 센서
    elif sensor_id == '00':
        return 2  # EEPROM 미설정 센서
    else:
        return 3  # 기타/오류 센서
```

---

## 📊 데이터 플로우

### 1. 센서 데이터 수신 및 처리

```mermaid
graph TD
    A[Arduino JSON 데이터 수신] --> B[SensorData.from_json()]
    B --> C[우선순위 그룹 결정]
    C --> D[센서 딕셔너리에 저장/업데이트]
    D --> E[타임스탬프 업데이트]
    E --> F[대시보드 표시 준비]
```

### 2. 정렬 및 표시 처리

```mermaid
graph TD
    A[get_sorted_sensors() 호출] --> B[모든 센서 데이터 수집]
    B --> C[정렬 키 함수 적용]
    C --> D[우선순위별 정렬 실행]
    D --> E[정렬된 센서 리스트 반환]
    E --> F[최대 8개까지 대시보드 표시]
```

---

## 🎨 대시보드 표시 포맷

### 주소 포맷팅
16자리 고유주소를 4-4-4-4 패턴으로 가독성 향상:
```
원본: 285882840000000E
표시: 2858:8284:0000:000E
```

### 표시 정보 구조

```python
display_info = {
    'sensor_id': '01',
    'temperature': 27.8,
    'address': '28E79B850000002D',
    'formatted_address': '28E7:9B85:0000:002D',
    'user_id': 260,
    'th_value': 30,
    'tl_value': 15,
    'priority_group': 1,
    'group_name': '설정됨',
    'is_configured': True
}
```

---

## 🧪 테스트 시나리오

### 테스트 데이터 구성
```python
test_sensors = [
    # 2순위: EEPROM 미설정 (sensor_id: 00)
    {"sensor_id": "00", "sensor_addr": "285882840000000E", "temperature": 28.5},
    {"sensor_id": "00", "sensor_addr": "285C82850000005D", "temperature": 28.1},
    {"sensor_id": "00", "sensor_addr": "28E6AA830000005A", "temperature": 28.2},
    
    # 1순위: EEPROM 설정됨 (sensor_id: 01-08)
    {"sensor_id": "03", "sensor_addr": "28FF641F43B82384", "temperature": 29.5},
    {"sensor_id": "01", "sensor_addr": "28E79B850000002D", "temperature": 27.8},
    {"sensor_id": "05", "sensor_addr": "28AA1B2C3D4E5F60", "temperature": 30.1},
]
```

### 예상 정렬 결과
```
1. [설정됨] ID:01, 27.8°C, Addr:28E79B850000002D
2. [설정됨] ID:03, 29.5°C, Addr:28FF641F43B82384  
3. [설정됨] ID:05, 30.1°C, Addr:28AA1B2C3D4E5F60
4. [미설정] ID:00, 28.5°C, Addr:285882840000000E
5. [미설정] ID:00, 28.1°C, Addr:285C82850000005D
6. [미설정] ID:00, 28.2°C, Addr:28E6AA830000005A
```

---

## 🔧 구현 세부사항

### 메모리 관리
```python
# 오래된 센서 데이터 자동 정리 (연결 해제된 센서)
def clear_old_sensors(self, max_age_seconds: int = 10) -> int:
    current_time = self._last_update_time
    to_remove = []
    
    for addr, sensor in self._sensors.items():
        if current_time - sensor.timestamp > max_age_seconds * 1000:
            to_remove.append(addr)
    
    for addr in to_remove:
        del self._sensors[addr]
    
    return len(to_remove)
```

### 그룹별 센서 분류
```python
def get_sensors_by_group(self) -> Dict[int, List[SensorData]]:
    """그룹별로 분류된 센서 딕셔너리"""
    groups = {1: [], 2: [], 3: []}
    
    for sensor in self.get_sorted_sensors():
        groups[sensor.priority_group].append(sensor)
    
    return groups
```

---

## 📊 성능 특성

### 시간 복잡도
- **센서 추가**: O(1)
- **정렬**: O(n log n) - n은 센서 개수
- **그룹별 분류**: O(n)
- **표시 데이터 생성**: O(min(n, 8))

### 공간 복잡도
- **센서 저장**: O(n) - 센서 개수에 비례
- **정렬 결과**: O(n) - 임시 리스트 생성
- **표시 데이터**: O(8) - 최대 8개 센서만 반환

---

## 🎯 사용 예시

### 기본 사용법
```python
# 센서 데이터 매니저 초기화
manager = SensorDataManager()

# Arduino에서 센서 데이터 수신 시
sensor_json = {
    "type": "sensor_data",
    "temperature": 28.5,
    "sensor_addr": "285882840000000E", 
    "sensor_id": "00",
    "user_sensor_id": 1281,
    "th_value": 30,
    "tl_value": 15,
    "timestamp": 1000
}

# 센서 데이터 추가
manager.add_sensor_data(sensor_json)

# 정렬된 센서 목록 얻기
sorted_sensors = manager.get_sorted_sensors()

# 대시보드 표시용 정보 얻기 (최대 8개)
display_info = manager.get_display_info(max_sensors=8)
```

### 실시간 업데이트
```python
# 주기적으로 오래된 센서 정리 (연결 해제 감지)
removed_count = manager.clear_old_sensors(max_age_seconds=10)

# 센서 현황 요약
summary = manager.get_summary()
print(f"전체: {summary['total_sensors']}개")
print(f"설정됨: {summary['configured_sensors']}개") 
print(f"미설정: {summary['unconfigured_sensors']}개")
```

---

## 🚀 확장 계획

### 향후 개선사항
1. **센서 그룹 설정 저장**
   - 사용자가 설정한 센서 그룹을 파일에 저장
   - 재시작 시 설정 복원

2. **실시간 정렬 최적화**
   - 증분 정렬을 통한 성능 개선
   - 변경된 센서만 재정렬

3. **고급 필터링**
   - 온도 범위 필터
   - 센서 상태별 필터
   - 사용자 정의 그룹 필터

---

## 📁 관련 파일

- **구현**: `src/python/services/sensor_data_manager.py`
- **테스트**: `test_sensor_sorting.py`
- **사용 예시**: Arduino 연결 후 실시간 센서 데이터 수신

**다음 단계**: 대시보드 UI와 센서 정렬 시스템 연동