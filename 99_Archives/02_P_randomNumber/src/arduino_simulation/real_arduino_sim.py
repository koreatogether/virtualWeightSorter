"""
Real Arduino Implementation Simulator
실제 Arduino C++ 코드를 Python으로 시뮬레이션하는 확장 시뮬레이터

주요 기능:
- 8가지 실제 Arduino 구현 방식 지원
- 재귀, 배열, switch, 함수포인터, 삼항연산자, 람다, static, 비트연산
- Arduino 제약 조건 검증
- 성능 및 메모리 사용량 정확한 측정
"""

import os
import random
import time
from typing import Any, Dict

from arduino_mock import ArduinoUnoR4WiFiMock


class RealArduinoImplementationGenerator:
    """실제 Arduino 구현 방식을 시뮬레이션하는 생성기"""

    def __init__(self, impl_config: Dict[str, Any], arduino: ArduinoUnoR4WiFiMock):
        self.config = impl_config
        self.arduino = arduino
        self.type = impl_config.get("type", "unknown")
        self.impl_id = impl_config.get("id", "unknown")

        # 각 구현별 상태 변수 초기화
        self.prev_num = -1
        self.recursion_depth = 0
        self.max_recursion_depth = 100  # 재귀 깊이 제한

        # 함수 포인터 시뮬레이션용
        if self.type == "function_pointer":
            self.function_map = {
                0: self._get_num_0,
                1: self._get_num_1,
                2: self._get_num_2,
            }

        print(f"Real Arduino Implementation: {impl_config['name']} initialized")

    def generate_number(self, previous: int = None) -> int:
        """구현 타입에 따른 실제 Arduino 로직 시뮬레이션"""
        if previous is not None:
            self.prev_num = previous

        try:
            if self.type == "recursive":
                return self._recursive_method()
            elif self.type == "array_based":
                return self._array_conditional_method()
            elif self.type == "switch_based":
                return self._switch_case_method()
            elif self.type == "function_pointer":
                return self._function_pointer_method()
            elif self.type == "ternary_based":
                return self._ternary_formula_method()
            elif self.type == "lambda_based":
                return self._lambda_function_method()
            elif self.type == "static_based":
                return self._static_variable_method()
            elif self.type == "bitwise_based":
                return self._bitwise_operation_method()
            else:
                # 기본값으로 삼항 연산자 방식 사용
                return self._ternary_formula_method()

        except Exception as e:
            print(f"Error in {self.impl_id}: {e}")
            # 안전한 기본값 반환
            return self._safe_fallback()

    def _recursive_method(self) -> int:
        """
        재귀 함수 방식 시뮬레이션
        int getRandomNum1(){
          int num = random(0, 3);
          if (num == prevNum1){
            return getRandomNum1();
          }
          prevNum1 = num;
          return num;
        }
        """
        self.recursion_depth += 1

        # 무한 재귀 방지
        if self.recursion_depth > self.max_recursion_depth:
            self.recursion_depth = 0
            return (self.prev_num + 1) % 3

        # Arduino random(0, 3) 시뮬레이션
        num = self.arduino.random_range(0, 3)

        # 재귀 조건 검사
        if num == self.prev_num:
            return self._recursive_method()  # 재귀 호출

        self.prev_num = num
        self.recursion_depth = 0
        return num

    def _array_conditional_method(self) -> int:
        """
        배열과 조건문 방식
        int getRandomNum2(){
          int nums[3] = {0, 1, 2};
          int idx = random(0, 3);
          int num = nums[idx];
          if (num == prevNum2){
            idx = (idx + 1) % 3;
            num = nums[idx];
          }
          prevNum2 = num;
          return num;
        }
        """
        # 배열 시뮬레이션
        nums = [0, 1, 2]
        idx = self.arduino.random_range(0, 3)
        num = nums[idx]

        # 조건문 검사
        if num == self.prev_num:
            idx = (idx + 1) % 3
            num = nums[idx]

        self.prev_num = num
        return num

    def _switch_case_method(self) -> int:
        """
        Switch문 방식
        switch (num){
          case 0: if (prevNum3 == 0) num = 1; break;
          case 1: if (prevNum3 == 1) num = 2; break;
          case 2: if (prevNum3 == 2) num = 0; break;
        }
        """
        num = self.arduino.random_range(0, 3)

        # Switch 문 시뮬레이션
        if num == 0:
            if self.prev_num == 0:
                num = 1
        elif num == 1:
            if self.prev_num == 1:
                num = 2
        elif num == 2:
            if self.prev_num == 2:
                num = 0

        self.prev_num = num
        return num

    def _function_pointer_method(self) -> int:
        """
        함수 포인터 방식
        int (*getNumFuncs[3])() = {getNum0, getNum1, getNum2};
        int getRandomNum4(){
          int idx = random(0, 3);
          int num = getNumFuncs[idx]();
          prevNum4 = num;
          return num;
        }
        """
        idx = self.arduino.random_range(0, 3)
        num = self.function_map[idx]()  # 함수 포인터 호출 시뮬레이션

        self.prev_num = num
        return num

    def _get_num_0(self) -> int:
        """getNum0() 함수 시뮬레이션"""
        return 1 if self.prev_num == 0 else 0

    def _get_num_1(self) -> int:
        """getNum1() 함수 시뮬레이션"""
        return 2 if self.prev_num == 1 else 1

    def _get_num_2(self) -> int:
        """getNum2() 함수 시뮬레이션"""
        return 0 if self.prev_num == 2 else 2

    def _ternary_formula_method(self) -> int:
        """
        삼항 연산자와 수식 방식
        int getRandomNum5(){
          int num = random(0, 3);
          num = (num == prevNum5) ? ((num + 1) % 3) : num;
          prevNum5 = num;
          return num;
        }
        """
        num = self.arduino.random_range(0, 3)

        # 삼항 연산자 시뮬레이션
        num = ((num + 1) % 3) if (num == self.prev_num) else num

        self.prev_num = num
        return num

    def _lambda_function_method(self) -> int:
        """
        람다 함수 방식 (C++11)
        auto pick = [](int prev){
          int n = random(0, 3);
          if (n == prev)
            n = (n + 2) % 3;
          return n;
        };
        """

        # 람다 함수 시뮬레이션
        def pick(prev):
            n = self.arduino.random_range(0, 3)
            if n == prev:
                n = (n + 2) % 3
            return n

        num = pick(self.prev_num)
        self.prev_num = num
        return num

    def _static_variable_method(self) -> int:
        """
        Static 변수 방식
        int getRandomNum7(){
          static int prevNum7 = -1;
          int num = random(0, 3);
          if (num == prevNum7)
            num = (num + 2) % 3;
          prevNum7 = num;
          return num;
        }
        """
        # static 변수는 인스턴스 변수로 시뮬레이션
        num = self.arduino.random_range(0, 3)

        if num == self.prev_num:
            num = (num + 2) % 3

        self.prev_num = num
        return num

    def _bitwise_operation_method(self) -> int:
        """
        비트 연산 방식
        int getRandomNum8(){
          int num = random(0, 3);
          if ((num ^ prevNum8) == 0)
            num = (num + 1) % 3;
          prevNum8 = num;
          return num;
        }
        """
        num = self.arduino.random_range(0, 3)

        # XOR 비트 연산 시뮬레이션
        if (num ^ self.prev_num) == 0:  # 같은 숫자면 XOR 결과가 0
            num = (num + 1) % 3

        self.prev_num = num
        return num

    def _safe_fallback(self) -> int:
        """안전한 기본값 반환"""
        candidates = [0, 1, 2]
        if self.prev_num in candidates:
            candidates.remove(self.prev_num)

        if candidates:
            return random.choice(candidates)
        else:
            return 0

    def get_implementation_stats(self) -> Dict[str, Any]:
        """구현별 통계 정보 반환"""
        return {
            "implementation_id": self.impl_id,
            "implementation_name": self.config.get("name", "Unknown"),
            "type": self.type,
            "recursion_depth_used": self.recursion_depth,
            "expected_performance": self.config.get("expected_performance", "unknown"),
            "expected_memory": self.config.get("memory_usage", "unknown"),
            "constraint_compliance": self.config.get(
                "constraint_compliance", "unknown"
            ),
            "arduino_code_lines": len(self.config.get("arduino_code", "").split("\n")),
            "cpp_version": self.config.get("cpp_version", "C++98"),
        }


def test_real_arduino_implementations():
    """실제 Arduino 구현들 테스트"""
    import yaml

    print("=== Real Arduino Implementations Test ===")

    # YAML 설정 로드
    try:
        # 설정 파일 경로 (프로젝트 루트의 config 폴더)
        config_path = os.path.join(
            os.path.dirname(__file__),
            "..",
            "..",
            "config",
            "arduino_implementations_real.yaml",
        )
        with open(config_path, encoding="utf-8") as f:
            config = yaml.safe_load(f)
    except FileNotFoundError:
        print("❌ arduino_implementations_real.yaml not found")
        print(f"Expected path: {config_path}")
        return

    implementations = config.get("implementations", [])
    test_iterations = 1000

    print(f"Testing {len(implementations)} real Arduino implementations")
    print(f"Iterations per implementation: {test_iterations:,}")
    print("-" * 60)

    results = []

    for impl in implementations:
        if not impl.get("enabled", True):
            continue

        print(f"\nTesting: {impl['name']}")
        print(f"Type: {impl['type']}")
        print(f"Description: {impl['description']}")

        try:
            # Arduino Mock 생성
            arduino = ArduinoUnoR4WiFiMock(seed=12345)
            generator = RealArduinoImplementationGenerator(impl, arduino)

            # 성능 측정
            start_time = time.time()
            generated_numbers = []
            violations = 0

            previous = -1
            for i in range(test_iterations):
                number = generator.generate_number(previous)
                generated_numbers.append(number)

                # 제약 조건 검사
                if previous != -1 and number == previous:
                    violations += 1

                previous = number

            end_time = time.time()
            execution_time = end_time - start_time
            generation_rate = (
                test_iterations / execution_time if execution_time > 0 else 0
            )

            # 분포 분석
            distribution = {i: generated_numbers.count(i) for i in range(3)}

            # 결과 저장
            result = {
                "name": impl["name"],
                "type": impl["type"],
                "generation_rate": generation_rate,
                "violations": violations,
                "distribution": distribution,
                "execution_time": execution_time,
                "stats": generator.get_implementation_stats(),
            }
            results.append(result)

            print(
                f"✅ Success: {generation_rate:,.0f} gen/sec, {violations} violations"
            )
            print(f"   Distribution: {distribution}")

        except Exception as e:
            print(f"❌ Failed: {e}")
            import traceback

            traceback.print_exc()

    # 결과 요약
    print(f"\n{'='*60}")
    print("SUMMARY RESULTS")
    print(f"{'='*60}")

    if results:
        # 성능 순으로 정렬
        results.sort(key=lambda x: x["generation_rate"], reverse=True)

        print(
            f"{'Rank':<4} {'Implementation':<25} {'Speed (gen/sec)':<15} {'Violations':<10}"
        )
        print("-" * 60)

        for i, result in enumerate(results, 1):
            print(
                f"{i:<4} {result['name']:<25} {result['generation_rate']:>10,.0f} {result['violations']:>10}"
            )

        # 최고 성능
        best = results[0]
        print(f"\n🏆 Best Performance: {best['name']}")
        print(f"   Speed: {best['generation_rate']:,.0f} gen/sec")
        print(f"   Violations: {best['violations']}")

        # 제약 조건 준수
        compliant = [r for r in results if r["violations"] == 0]
        print(
            f"\n✅ Constraint Compliant: {len(compliant)}/{len(results)} implementations"
        )

    else:
        print("No successful results")


if __name__ == "__main__":
    test_real_arduino_implementations()
