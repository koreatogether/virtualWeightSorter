블루투스 에는 크게 2가지 전송방식이 있다.
블루투스 클래스 , 블루투스 BLE 
클래스는 전력 소비가 많은 편이지만 많은 데이타를 전송가능하고 
BLE 는 전력 소비가 적지만 적은 데이타만 전송가능하다.

전송 속도 비교

2023 규격 기준 
CLASSIC : 48Mbps
BLE     :  2Mbps
고로 CLASSIC 과 BLE는 서로 통신이 안된다. 서로 다른 목적으로 나뉘어져서 그렇다.

BLE 전송방식 
BLE에서 단순히 데이타를 주기만 하는 모드가 있고 기기 끼리 연결을 해서 데이타를 주고 받는 연결 모드 2가지가 있다.

BLE 의 전력 소비 패턴
연결이 시작될때를 빼고는 항상 절전모드를 유지한다. ( 클래식 대비 100배 적은 전력 소모 )

기타
ble 소개를 해주고 있는 글 주소 : https://fishpoint.tistory.com/9804

BLE 계층적 데이터 구조를 정의
![alt text](image.png)

UUID는 SERVICE , CHARACTERISTIC , DESCRIPTOR만 갖는다.

GATT는 두 BLE 기기가 표준 메시지를 보내고 받는 방식을 정의

블루투스 관련 안내 사이트 
https://www.bluetooth.com/specifications/assigned-numbers/

UUID 생성 사이트
https://www.uuidgenerator.net/

BLE 구현에 문제가 생길한한 글 및 해결방법
https://talkingaboutme.tistory.com/entry/Arduino-Nano-BLE-problem
