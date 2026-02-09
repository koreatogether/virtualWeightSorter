import processing.serial.*;

Serial myPort; // 시리얼 통신을 위한 Serial 객체
String val; // 시리얼에서 읽은 값을 저장할 변수
int pitch = 0;

void setup() {
  size(400, 400);
  println(Serial.list()); // 사용 가능한 시리얼 포트 목록을 콘솔에 출력
  myPort = new Serial(this, Serial.list()[1], 9600); // 첫 번째 시리얼 포트를 9600bps로 연결
  myPort.bufferUntil('\n'); // 줄바꿈 문자를 기준으로 데이터를 읽음
}

void draw() {
  background(255);

  // 배너의 각도를 pitch와 roll 값에 따라 조정
  translate(width/2, height/2); // 회전의 중심을 캔버스의 중앙으로 이동
  rotate(radians(pitch)); // roll 값에 따라 캔버스 회전

  // 배너 그리기
  fill(255, 0, 0);
  rect(-100, -25, 200, 50); // 중앙을 기준으로 배너 그리기

  // 원래 상태로 복귀
  resetMatrix();
}

void serialEvent(Serial myPort) {
  String inString = myPort.readStringUntil('\n');
  if (inString != null) {
    inString = trim(inString); // 앞뒤 공백 제거
    pitch = int(inString); // inString을 정수로 변환하여 pitch에 할당
  }
}
