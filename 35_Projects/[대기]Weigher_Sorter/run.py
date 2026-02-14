#!/usr/bin/env python3
"""
UV 가상환경에서 Python 스크립트 실행을 위한 편의 스크립트
Usage: uv run run.py <script_name>
Example: uv run run.py test_runner
"""

import sys
import os
import subprocess
from pathlib import Path

def main():
    if len(sys.argv) < 2:
        print("Available scripts:")
        print("  test_runner    - 전체 테스트 실행")
        print("  simulator      - Arduino 시뮬레이터 테스트")
        print("  generators     - 랜덤 생성기 테스트")
        print("  algorithms     - 조합 알고리즘 테스트")
        print("  monitor        - 성능 모니터 테스트")
        print()
        print("Usage: uv run run.py <script_name>")
        return

    script_name = sys.argv[1]
    python_dir = Path("src/python")
    
    script_map = {
        "test_runner": "test_runner.py",
        "simulator": "arduino_r4_simulator.py", 
        "generators": "random_generators.py",
        "algorithms": "combination_algorithms.py",
        "monitor": "performance_monitor.py"
    }
    
    if script_name not in script_map:
        print(f"Unknown script: {script_name}")
        print("Use 'uv run run.py' to see available scripts")
        return
    
    script_path = python_dir / script_map[script_name]
    
    if not script_path.exists():
        print(f"Script not found: {script_path}")
        return
    
    # UV를 통해 스크립트 실행
    os.chdir(python_dir)
    subprocess.run([sys.executable, script_path])

if __name__ == "__main__":
    main()