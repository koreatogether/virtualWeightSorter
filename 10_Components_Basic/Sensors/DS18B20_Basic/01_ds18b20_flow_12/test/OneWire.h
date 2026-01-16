#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdint.h>

// OneWire 클래스의 최소한의 정의
class OneWire {
public:
    OneWire(uint8_t pin) {}
    // 필요한 경우 여기에 OneWire 라이브러리의 다른 함수들을 추가합니다.
    // 예를 들어, search, reset, read, write 등
};

#endif // ONEWIRE_H
