#ifndef DHT22_NEW_H
#define DHT22_NEW_H

#include <DHT.h>
#include "dht22_new_config.h"

// DHT22 센서 객체 선언
extern DHT dht22_sensor;

// DHT22 센서 초기화 함수
void init_dht22_sensor();

// DHT22 센서 메인 루프 처리 함수
void dht22_loop();

#endif // DHT22_NEW_H
