#include "FakeArduino.h"

// millis() 함수를 흉내 내는 가짜 함수
unsigned long _fake_millis_val = 0;
unsigned long millis() {
    return _fake_millis_val;
}
void setFakeMillis(unsigned long val) {
    _fake_millis_val = val;
}

// delay() 함수를 흉내 내는 가짜 함수 (실제 지연 없이)
void delay(unsigned long ms) {
    // 테스트에서는 실제 지연을 발생시키지 않습니다.
}
