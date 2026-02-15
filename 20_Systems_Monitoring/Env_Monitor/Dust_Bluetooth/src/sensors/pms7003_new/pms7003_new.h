#ifndef PMS7003_NEW_H
#define PMS7003_NEW_H

#include <PMS.h>
#include "pms7003_new_config.h"

// PMS7003 센서 객체 선언 (Hardware Serial 사용)
extern PMS pms7003_sensor;
extern PMS::DATA pms7003_data;

// PMS7003 센서 초기화 함수
void init_pms7003_sensor();

// PMS7003 센서 메인 루프 처리 함수
void pms7003_loop();

#endif // PMS7003_NEW_H