#!/usr/bin/env python3
"""
Repository root launcher moved to tools/scripts/main.py
"""
from pathlib import Path
import sys

project_root = Path(__file__).parents[2]
sys.path.insert(0, str(project_root))

def main():
    print("Hello from 05_P_ds18b20_monitoring (tools/scripts/main.py)")

if __name__ == "__main__":
    main()
