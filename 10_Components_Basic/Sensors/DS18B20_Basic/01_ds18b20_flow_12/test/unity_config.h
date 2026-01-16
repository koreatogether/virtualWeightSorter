#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

/* Unity 구성 옵션 */

/* 출력 문자 핸들러를 사용자 정의 함수로 재정의 */
#define UNITY_OUTPUT_CHAR(a) unity_custom_output_char(a)
#define UNITY_OUTPUT_FLUSH() unity_custom_output_flush()

/* 테스트 실행 전/후 및 각 테스트 케이스 전/후 호출될 콜백 */
#define UNITY_BEGIN() unity_custom_begin()
#define UNITY_END() unity_custom_end()

/* Unity 출력 핸들러 함수 선언 */
void unity_custom_output_char(int);
void unity_custom_output_flush(void);
int unity_custom_begin(void);
int unity_custom_end(void);

#endif /* UNITY_CONFIG_H */
