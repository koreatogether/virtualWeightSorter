"""
간단한 115200 bps 테스트 스크립트
"""
import sys
import os
import time

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

from src.python.services.port_manager import port_manager

def simple_test():
    print("=== 115200 bps Test ===")
    
    success = port_manager.connect("COM4")
    print(f"Connection: {success}")
    
    if success:
        comm_result = port_manager.test_communication()
        print(f"Communication: {comm_result}")
        
        status = port_manager.get_status()
        print(f"Status: {status}")
        
        port_manager.disconnect()
    
    print("Test complete")

if __name__ == "__main__":
    simple_test()
