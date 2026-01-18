#!/usr/bin/env python3
"""
Statistical Analysis for Arduino Implementations
조건부 확률 및 빈도 분석을 위한 상세 통계 시스템

주요 기능:
- 전체 빈도 분석 (Overall Frequencies)
- 조건부 확률 분석 (Conditional Probabilities)
- 편향성 검증 (Bias Detection)
- 통계적 유의성 검증
"""

import os
import sys
from collections import defaultdict
from typing import Any, Dict, List

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
import yaml

# 경로 추가
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))

from arduino_mock import ArduinoUnoR4WiFiMock
from real_arduino_sim import RealArduinoImplementationGenerator


class StatisticalAnalyzer:
    """Arduino 구현의 통계적 특성 분석기"""

    def __init__(self):
        self.implementations = self._load_implementations()
        self.results = {}

    def _load_implementations(self) -> List[Dict[str, Any]]:
        """구현 목록 로드"""
        try:
            config_path = os.path.join(
                os.path.dirname(__file__),
                "..",
                "..",
                "..",
                "config",
                "arduino_implementations_real.yaml",
            )
            with open(config_path, encoding="utf-8") as f:
                config = yaml.safe_load(f)
            return [
                impl
                for impl in config.get("implementations", [])
                if impl.get("enabled", True)
            ]
        except Exception as e:
            print(f"Error loading implementations: {e}")
            return []

    def analyze_all_implementations(
        self, iterations: int = 10000, seed: int = 12345
    ) -> Dict[str, Any]:
        """모든 구현에 대한 상세 통계 분석"""
        print(
            f"=== Statistical Analysis of {len(self.implementations)} Implementations ==="
        )
        print(f"Iterations: {iterations:,}, Seed: {seed}")
        print("-" * 70)

        all_results = {}

        for i, impl in enumerate(self.implementations, 1):
            print(f"\n[{i}/{len(self.implementations)}] Analyzing: {impl['name']}")

            try:
                # 시뮬레이션 실행
                arduino = ArduinoUnoR4WiFiMock(seed=seed)
                generator = RealArduinoImplementationGenerator(impl, arduino)

                # 숫자 생성
                generated_numbers = []
                previous = -1

                for _ in range(iterations):
                    number = generator.generate_number(previous)
                    generated_numbers.append(number)
                    previous = number

                # 통계 분석
                stats = self._analyze_sequence(generated_numbers, impl["name"])
                all_results[impl["id"]] = {
                    "name": impl["name"],
                    "type": impl["type"],
                    "sequence": generated_numbers,
                    "stats": stats,
                }

                # 결과 출력
                self._print_implementation_stats(impl["name"], stats)

            except Exception as e:
                print(f"❌ Error analyzing {impl['name']}: {e}")
                continue

        # 종합 분석
        self._generate_comprehensive_report(all_results)

        return all_results

    def _analyze_sequence(self, sequence: List[int], name: str) -> Dict[str, Any]:
        """시퀀스의 상세 통계 분석"""
        total = len(sequence)

        # 1. 전체 빈도 분석
        frequencies = {i: sequence.count(i) for i in range(3)}
        freq_percentages = {
            i: round(count / total, 3) for i, count in frequencies.items()
        }

        # 2. 조건부 확률 분석
        transitions = defaultdict(lambda: defaultdict(int))

        for i in range(1, len(sequence)):
            prev = sequence[i - 1]
            curr = sequence[i]
            transitions[prev][curr] += 1

        # 조건부 확률 계산
        conditional_probs = {}
        for prev in range(3):
            total_from_prev = sum(transitions[prev].values())
            if total_from_prev > 0:
                probs = [
                    round(transitions[prev][next_val] / total_from_prev, 3)
                    for next_val in range(3)
                ]
                conditional_probs[prev] = probs
            else:
                conditional_probs[prev] = [0.0, 0.0, 0.0]

        # 3. 편향성 분석
        bias_analysis = self._analyze_bias(conditional_probs)

        # 4. 제약 조건 검증
        violations = sum(
            1 for i in range(1, len(sequence)) if sequence[i] == sequence[i - 1]
        )

        # 5. 균등성 검증 (카이제곱 검정)
        expected = total / 3
        chi_square = sum(
            (count - expected) ** 2 / expected for count in frequencies.values()
        )

        return {
            "total_count": total,
            "frequencies": frequencies,
            "freq_percentages": freq_percentages,
            "conditional_probs": conditional_probs,
            "bias_analysis": bias_analysis,
            "violations": violations,
            "chi_square": chi_square,
            "transitions": dict(transitions),
        }

    def _analyze_bias(
        self, conditional_probs: Dict[int, List[float]]
    ) -> Dict[str, Any]:
        """편향성 분석"""
        bias_results = {}

        for prev in range(3):
            probs = conditional_probs[prev]
            # 대각선 확률은 0이어야 함 (제약 조건)
            non_zero_probs = [p for i, p in enumerate(probs) if i != prev and p > 0]

            if len(non_zero_probs) == 2:
                p1, p2 = non_zero_probs
                ratio = max(p1, p2) / min(p1, p2) if min(p1, p2) > 0 else float("inf")

                if abs(p1 - 0.5) < 0.05 and abs(p2 - 0.5) < 0.05:
                    bias_type = "균등 (≈0.5/0.5)"
                elif ratio > 1.8:  # 약 2/3 : 1/3 비율
                    if p1 > p2:
                        bias_type = f"편향 ({p1:.3f}:{p2:.3f})"
                    else:
                        bias_type = f"편향 ({p2:.3f}:{p1:.3f})"
                else:
                    bias_type = f"약간 편향 ({p1:.3f}:{p2:.3f})"

                bias_results[f"prev_{prev}"] = {
                    "type": bias_type,
                    "ratio": ratio,
                    "probs": non_zero_probs,
                }

        return bias_results

    def _print_implementation_stats(self, name: str, stats: Dict[str, Any]):
        """구현별 통계 출력"""
        print(f"  Overall Frequencies: {stats['freq_percentages']}")
        print("  Conditional Probabilities:")
        for prev in range(3):
            probs = stats["conditional_probs"][prev]
            bias_info = stats["bias_analysis"].get(f"prev_{prev}", {})
            bias_type = bias_info.get("type", "Unknown")
            print(f"    Prev {prev}: {probs} - {bias_type}")
        print(f"  Violations: {stats['violations']}")
        print(f"  Chi-square: {stats['chi_square']:.3f}")

    def _generate_comprehensive_report(self, results: Dict[str, Any]):
        """종합 보고서 생성"""
        print(f"\n{'='*70}")
        print("COMPREHENSIVE STATISTICAL REPORT")
        print(f"{'='*70}")

        # 1. 전체 빈도 표
        print("\n📊 전체 빈도 (Overall Frequencies)")
        print("=" * 50)
        freq_data = []
        for impl_id, data in results.items():
            name = data["name"]
            freqs = data["stats"]["freq_percentages"]
            freq_data.append([name[:20], freqs[0], freqs[1], freqs[2]])

        freq_df = pd.DataFrame(
            freq_data, columns=["Implementation", "0의 빈도", "1의 빈도", "2의 빈도"]
        )
        print(freq_df.to_string(index=False, float_format="%.3f"))

        # 2. 조건부 확률 표
        print("\n🎯 조건부 확률 (Conditional Probabilities)")
        print("=" * 80)
        cond_data = []
        for impl_id, data in results.items():
            name = data["name"]
            cond_probs = data["stats"]["conditional_probs"]
            bias_analysis = data["stats"]["bias_analysis"]

            # 편향성 종합 판단
            bias_types = [info.get("type", "") for info in bias_analysis.values()]
            if all("균등" in bt for bt in bias_types):
                overall_bias = "균등 (≈0.5/0.5)"
            elif any("편향" in bt for bt in bias_types):
                overall_bias = "편향 존재"
            else:
                overall_bias = "혼합"

            cond_data.append(
                [
                    name[:15],
                    str(cond_probs[0]),
                    str(cond_probs[1]),
                    str(cond_probs[2]),
                    overall_bias,
                ]
            )

        cond_df = pd.DataFrame(
            cond_data,
            columns=[
                "Implementation",
                "Prev 0: [0,1,2]",
                "Prev 1: [0,1,2]",
                "Prev 2: [0,1,2]",
                "편향성",
            ],
        )
        print(cond_df.to_string(index=False))

        # 3. 통계적 유의성 분석
        print("\n📈 통계적 유의성 분석")
        print("=" * 50)

        chi_squares = [data["stats"]["chi_square"] for data in results.values()]
        violations = [data["stats"]["violations"] for data in results.values()]

        print(f"카이제곱 통계량 범위: {min(chi_squares):.3f} ~ {max(chi_squares):.3f}")
        print(f"평균 카이제곱: {np.mean(chi_squares):.3f}")
        print(
            f"제약 조건 위반: 총 {sum(violations)}개 (평균 {np.mean(violations):.1f}개/구현)"
        )

        # 4. 편향성 패턴 분석
        self._analyze_bias_patterns(results)

    def _analyze_bias_patterns(self, results: Dict[str, Any]):
        """편향성 패턴 분석"""
        print("\n🔍 편향성 패턴 분석")
        print("=" * 50)

        pattern_counts = defaultdict(int)

        for impl_id, data in results.items():
            bias_analysis = data["stats"]["bias_analysis"]

            # 각 구현의 편향 패턴 분류
            uniform_count = sum(
                1 for info in bias_analysis.values() if "균등" in info.get("type", "")
            )
            biased_count = sum(
                1 for info in bias_analysis.values() if "편향" in info.get("type", "")
            )

            if uniform_count == 3:
                pattern = "완전 균등"
            elif biased_count == 3:
                pattern = "완전 편향"
            else:
                pattern = "혼합"

            pattern_counts[pattern] += 1

            print(f"{data['name'][:20]:20} - {pattern}")

        print("\n패턴 요약:")
        for pattern, count in pattern_counts.items():
            print(f"  {pattern}: {count}개 구현")

    def generate_visualization(self, results: Dict[str, Any]):
        """시각화 생성"""
        print("\n📊 시각화 생성 중...")

        # 1. 전체 빈도 히트맵
        freq_data = []
        impl_names = []

        for impl_id, data in results.items():
            impl_names.append(data["name"][:15])
            freqs = data["stats"]["freq_percentages"]
            freq_data.append([freqs[0], freqs[1], freqs[2]])

        plt.figure(figsize=(12, 8))

        # 서브플롯 1: 전체 빈도
        plt.subplot(2, 2, 1)
        sns.heatmap(
            freq_data,
            xticklabels=["0", "1", "2"],
            yticklabels=impl_names,
            annot=True,
            fmt=".3f",
            cmap="RdYlBu_r",
            center=0.333,
        )
        plt.title("Overall Frequencies")
        plt.xlabel("Number")
        plt.ylabel("Implementation")

        # 서브플롯 2: 카이제곱 통계량
        plt.subplot(2, 2, 2)
        chi_squares = [data["stats"]["chi_square"] for data in results.values()]
        plt.bar(range(len(impl_names)), chi_squares)
        plt.xticks(range(len(impl_names)), impl_names, rotation=45, ha="right")
        plt.title("Chi-square Statistics")
        plt.ylabel("Chi-square Value")

        # 서브플롯 3: 조건부 확률 (Prev=0 기준)
        plt.subplot(2, 2, 3)
        prev0_data = []
        for impl_id, data in results.items():
            probs = data["stats"]["conditional_probs"][0]
            prev0_data.append([probs[1], probs[2]])  # 0은 항상 0이므로 제외

        sns.heatmap(
            prev0_data,
            xticklabels=["P(1|0)", "P(2|0)"],
            yticklabels=impl_names,
            annot=True,
            fmt=".3f",
            cmap="RdYlGn",
            center=0.5,
        )
        plt.title("Conditional Probabilities (Prev=0)")

        # 서브플롯 4: 편향성 비율
        plt.subplot(2, 2, 4)
        bias_ratios = []
        for impl_id, data in results.items():
            bias_analysis = data["stats"]["bias_analysis"]
            ratios = [info.get("ratio", 1.0) for info in bias_analysis.values()]
            avg_ratio = np.mean([r for r in ratios if r != float("inf")])
            bias_ratios.append(avg_ratio)

        plt.bar(range(len(impl_names)), bias_ratios)
        plt.axhline(y=2.0, color="r", linestyle="--", alpha=0.7, label="2:1 Ratio")
        plt.xticks(range(len(impl_names)), impl_names, rotation=45, ha="right")
        plt.title("Average Bias Ratio")
        plt.ylabel("Ratio")
        plt.legend()

        plt.tight_layout()
        plt.savefig("statistical_analysis.png", dpi=300, bbox_inches="tight")
        plt.show()

        print("시각화 저장됨: statistical_analysis.png")

    def export_detailed_report(
        self, results: Dict[str, Any], filename: str = "detailed_statistical_report.txt"
    ):
        """상세 보고서 텍스트 파일로 출력"""
        with open(filename, "w", encoding="utf-8") as f:
            f.write("Arduino Random Number Generator - Detailed Statistical Analysis\n")
            f.write("=" * 70 + "\n\n")

            # 전체 빈도 표
            f.write("전체 빈도 (Overall Frequencies):\n")
            f.write("-" * 50 + "\n")
            f.write(
                f"{'코드 번호':<15} {'0의 빈도':<10} {'1의 빈도':<10} {'2의 빈도':<10}\n"
            )

            for i, (impl_id, data) in enumerate(results.items(), 1):
                freqs = data["stats"]["freq_percentages"]
                f.write(
                    f"{i:<15} {freqs[0]:<10.3f} {freqs[1]:<10.3f} {freqs[2]:<10.3f}\n"
                )

            # 조건부 확률 표
            f.write("\n조건부 확률 (Conditional Probabilities):\n")
            f.write("-" * 80 + "\n")
            f.write(
                f"{'코드 번호':<10} {'Prev 0: [0,1,2] 확률':<25} {'Prev 1: [0,1,2] 확률':<25} {'Prev 2: [0,1,2] 확률':<25} {'균등 여부':<15}\n"
            )

            for i, (impl_id, data) in enumerate(results.items(), 1):
                cond_probs = data["stats"]["conditional_probs"]
                bias_analysis = data["stats"]["bias_analysis"]

                # 편향성 판단
                bias_types = [info.get("type", "") for info in bias_analysis.values()]
                if all("균등" in bt for bt in bias_types):
                    bias_summary = "균등 (≈0.5/0.5)"
                else:
                    bias_summary = "편향"

                f.write(
                    f"{i:<10} {str(cond_probs[0]):<25} {str(cond_probs[1]):<25} {str(cond_probs[2]):<25} {bias_summary:<15}\n"
                )

        print(f"상세 보고서 저장됨: {filename}")


def main():
    """메인 실행 함수"""
    print("Arduino Implementation Statistical Analysis")
    print("=" * 60)

    analyzer = StatisticalAnalyzer()

    # 분석 실행
    results = analyzer.analyze_all_implementations(iterations=10000, seed=12345)

    if results:
        # 시각화 생성
        try:
            analyzer.generate_visualization(results)
        except Exception as e:
            print(f"시각화 생성 실패: {e}")

        # 상세 보고서 출력
        analyzer.export_detailed_report(results)

        print("\n🎉 분석 완료!")
        print(f"📊 총 {len(results)}개 구현 분석")
        print("📈 시각화: statistical_analysis.png")
        print("📄 상세 보고서: detailed_statistical_report.txt")
    else:
        print("❌ 분석할 구현이 없습니다.")


if __name__ == "__main__":
    main()
