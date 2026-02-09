import processing.serial.*; // 시리얼 라이브러리 임포트

Serial myPort1, myPort2, myPort3; // 시리얼 포트 객체
int val1, val2, val3; // 수신된 데이터를 저장할 변수

void setup() {
  size(400, 300); // 창 크기 설정

  // 시리얼 포트 열기
  printArray(Serial.list()); // 사용 가능한 시리얼 포트 목록 출력
  String portName1 = Serial.list()[0]; // 첫 번째 시리얼 포트 선택
  String portName2 = Serial.list()[1]; // 두 번째 시리얼 포트 선택
  String portName3 = Serial.list()[2]; // 세 번째 시리얼 포트 선택

  myPort1 = new Serial(this, portName1, 9600); // 시리얼 포트 열기
  myPort2 = new Serial(this, portName2, 9600);
  myPort3 = new Serial(this, portName3, 9600);
}

void draw() {
  if (myPort1.available() > 0) { // 데이터가 수신되었는지 확인
    val1 = myPort1.read(); // 수신된 데이터 읽기
    text("Value 1: " + val1, 10, 20); // 값 출력
  }

  if (myPort2.available() > 0) {
    val2 = myPort2.read();
    text("Value 2: " + val2, 10, 40); // 값 출력
  }

  if (myPort3.available() > 0) {
    val3 = myPort3.read();
    text("Value 3: " + val3, 10, 60); // 값 출력
  }
}
