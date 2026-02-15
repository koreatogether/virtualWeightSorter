
# 아두이노 기반 재고 관리 GitHub 프로젝트 목록

`02_구상` 폴더의 아이디어를 기반으로, 유사한 기능을 가진 GitHub 프로젝트들을 아래와 같이 정리했습니다.

## 1. RFID 기반 시스템

RFID 태그를 부품에 부착하여 재고를 관리하는 방식이 가장 일반적입니다. 아두이노는 태그를 읽는 역할을 하고, 데이터 처리 및 저장은 라즈베리파이나 클라우드 플랫폼에서 담당합니다.

### 추천 프로젝트

*   **[IoT-Based-RFID-Inventory-Management-System](https://github.com/Iot-based-rfid-inventory-management-system/Iot-based-rfid-inventory-management-system)**
    *   **설명:** MFRC522 RFID 리더와 아두이노 나노를 사용하여 태그를 스캔하고, NodeMCU(ESP8266)를 통해 Adafruit.io 클라우드로 데이터를 전송합니다. 실시간 재고 추적이 가능합니다.
    *   **특징:** 클라우드 기반, 실시간 시각화
    *   **기술 스택:** Arduino, NodeMCU (ESP8266), MFRC522, Adafruit.io

*   **[EAL - RFID Inventory Management](https://www.instructables.com/EAL-RFID-Inventory-Management/)**
    *   **설명:** 아두이노와 MFRC522 RFID 리더를 라즈베리파이에 연결하여 MySQL 데이터베이스에 재고 데이터를 저장합니다. Windows Forms 애플리케이션으로 데이터를 관리합니다.
    *   **특징:** 로컬 데이터베이스, 데스크톱 애플리케이션
    *   **기술 스택:** Arduino, Raspberry Pi, MFRC522, MySQL, Windows Forms

## 2. 바코드 기반 시스템

바코드 스캐너를 활용하는 방식도 가능합니다.

### 추천 프로젝트

*   **[Inventory-Management-System](https://github.com/Samael3003/Inventory-Management-System)**
    *   **설명:** 라즈베리파이에 바코드 스캐너를 연결하여 재고를 관리하는 시스템입니다. 아두이노는 환경 모니터링, NodeMCU는 재고 관리를 담당하는 등 모듈식 접근 방식을 사용합니다.
    *   **특징:** 모듈식 설계, 바코드 기반
    *   **기술 스택:** Raspberry Pi, Arduino, NodeMCU, Barcode Scanner

## 3. 핵심 부품 및 기술

위 프로젝트들에서 공통적으로 사용되는 핵심 부품과 기술은 다음과 같습니다.

*   **마이크로컨트롤러:** 아두이노 (센서 인터페이스), 라즈베리파이/ESP8266 (데이터 처리 및 인터넷 연결)
*   **센서:** RFID 리더, 바코드 스캐너
*   **데이터베이스:** MySQL, 클라우드 기반 DB (Adafruit.io 등)
*   **사용자 인터페이스:** 웹 기반 대시보드, 데스크톱 애플리케이션

## 4. 시작 가이드

*   **클라우드 기반으로 시작:** [IoT-Based-RFID-Inventory-Management-System](https://github.com/Iot-based-rfid-inventory-management-system/Iot-based-rfid-inventory-management-system) 프로젝트는 하드웨어 목록과 소스 코드가 잘 정리되어 있어 시작하기 좋습니다.
*   **로컬 데이터베이스 구축:** [EAL - RFID Inventory Management](https://www.instructables.com/EAL-RFID-Inventory-Management/)는 로컬 환경에 데이터베이스를 구축하고 싶을 때 참고하기 좋은 튜토리얼입니다.
