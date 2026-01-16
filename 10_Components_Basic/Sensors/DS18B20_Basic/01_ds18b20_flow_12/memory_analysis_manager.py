#!/usr/bin/env python3
"""
메모리 분석 기능 활성화/비활성화 스크립트
사용법: python memory_analysis_manager.py [enable|disable]
"""

import os
import sys
import shutil
import subprocess

# 스크립트 경로 설정
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.abspath(os.path.join(SCRIPT_DIR))

# 메모리 분석 관련 파일 목록
MEMORY_FILES = [
    "MemoryUsageTester.cpp",
    "MemoryUsageTester.h"
]

# 백업 폴더 설정
BACKUP_DIR = os.path.join(PROJECT_DIR, "memory_analysis_disabled")

def ensure_backup_dir():
    """백업 디렉토리가 없으면 생성"""
    if not os.path.exists(BACKUP_DIR):
        os.makedirs(BACKUP_DIR)
        print(f"백업 디렉토리 생성됨: {BACKUP_DIR}")

def enable_memory_analysis():
    """메모리 분석 기능 활성화"""
    print("메모리 분석 기능 활성화 중...")
    
    # 백업 폴더에서 원본 파일 복원
    for file in MEMORY_FILES:
        backup_file = os.path.join(BACKUP_DIR, f"{file}.orig")
        target_file = os.path.join(PROJECT_DIR, file)
        
        if os.path.exists(backup_file):
            # 기존 파일이 있으면 백업
            if os.path.exists(target_file):
                os.rename(target_file, os.path.join(BACKUP_DIR, f"{file}.empty"))
            
            # 원본 파일 복사
            shutil.copy2(backup_file, target_file)
            print(f"파일 복원됨: {file}")
        else:
            print(f"경고: 원본 파일이 없습니다: {backup_file}")
    
    # exclude_memory_analysis.h 파일 수정
    exclude_file = os.path.join(PROJECT_DIR, "exclude_memory_analysis.h")
    if os.path.exists(exclude_file):
        with open(exclude_file, 'r', encoding='utf-8') as f:
            content = f.read()
        # MEMORY_ANALYSIS_ENABLED 값을 1로 변경
        content = content.replace("#define MEMORY_ANALYSIS_ENABLED 0", "#define MEMORY_ANALYSIS_ENABLED 1")
        with open(exclude_file, 'w', encoding='utf-8') as f:
            f.write(content)
        print("exclude_memory_analysis.h 파일 업데이트됨: MEMORY_ANALYSIS_ENABLED = 1")
    
    print("메모리 분석 기능이 활성화되었습니다.")
    print("이제 'arduino-cli compile' 명령으로 프로젝트를 다시 컴파일하세요.")

def disable_memory_analysis():
    """메모리 분석 기능 비활성화"""
    print("메모리 분석 기능 비활성화 중...")
    ensure_backup_dir()
    
    # 원본 파일 백업 및 빈 파일로 대체
    for file in MEMORY_FILES:
        source_file = os.path.join(PROJECT_DIR, file)
        backup_file = os.path.join(BACKUP_DIR, f"{file}.orig")
        empty_file = os.path.join(BACKUP_DIR, f"{file}.empty")
        
        if os.path.exists(source_file):
            # 아직 백업이 없으면 백업 생성
            if not os.path.exists(backup_file):
                shutil.copy2(source_file, backup_file)
                print(f"원본 파일 백업됨: {file} -> {backup_file}")
            
            # 소스 파일을 빈 구현으로 대체
            if os.path.exists(empty_file):
                shutil.copy2(empty_file, source_file)
            else:
                # 빈 파일 생성
                with open(source_file, 'w') as f:
                    f.write(f"// 비활성화된 파일: {file}\n")
                    if file.endswith(".cpp"):
                        f.write('#include "MemoryUsageTester.h"\n\n// 메모리 분석 기능 비활성화됨\n')
                    elif file.endswith(".h"):
                        f.write("#ifndef MEMORY_USAGE_TESTER_H\n#define MEMORY_USAGE_TESTER_H\n\n")
                        f.write("// 메모리 분석 기능 비활성화됨\n")
                        f.write("class MemoryUsageTester {\npublic:\n")
                        f.write("    inline int freeMemory() { return 0; }\n")
                        f.write("    inline void runBasicTest() {}\n")
                        f.write("    inline void runStressTest(int = 0) {}\n")
                        f.write("    inline void printMemoryStatus(const char*) {}\n")
                        f.write("    inline void testDataStructures() {}\n")
                        f.write("    inline void printMemoryStatusCSV(const char*, const char*) {}\n")
                        f.write("    inline void logMemoryUsage() {}\n")
                        f.write("};\n\n#endif // MEMORY_USAGE_TESTER_H\n")
                
                print(f"파일이 빈 구현으로 대체됨: {file}")
        else:
            print(f"경고: 파일이 없습니다: {source_file}")
    
    # exclude_memory_analysis.h 파일 수정
    exclude_file = os.path.join(PROJECT_DIR, "exclude_memory_analysis.h")
    if os.path.exists(exclude_file):
        with open(exclude_file, 'r', encoding='utf-8') as f:
            content = f.read()
        # MEMORY_ANALYSIS_ENABLED 값을 0으로 변경
        content = content.replace("#define MEMORY_ANALYSIS_ENABLED 1", "#define MEMORY_ANALYSIS_ENABLED 0")
        with open(exclude_file, 'w', encoding='utf-8') as f:
            f.write(content)
        print("exclude_memory_analysis.h 파일 업데이트됨: MEMORY_ANALYSIS_ENABLED = 0")
    
    print("메모리 분석 기능이 비활성화되었습니다.")
    print("이제 'arduino-cli compile' 명령으로 프로젝트를 다시 컴파일하세요.")

def compile_project():
    """프로젝트 컴파일"""
    print("프로젝트 컴파일 중...")
    
    # arduino-cli 명령 실행
    cmd = ['arduino-cli', 'compile', '--fqbn', 'arduino:renesas_uno:unor4wifi', '.', '--output-dir', './build']
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_DIR)
        
        print("\n=== 컴파일 결과 ===")
        if result.returncode == 0:
            print("컴파일 성공!")
            
            # 메모리 사용량 출력 찾기
            for line in result.stdout.split('\n'):
                if "Sketch uses" in line or "Global variables use" in line:
                    print(line.strip())
        else:
            print("컴파일 실패!")
            print("에러 메시지:")
            print(result.stderr)
        
        return result.returncode
    except Exception as e:
        print(f"컴파일 중 오류 발생: {e}")
        return 1

def show_help():
    """도움말 표시"""
    print("메모리 분석 기능 관리 스크립트")
    print("사용법: python memory_analysis_manager.py [명령]")
    print("\n사용 가능한 명령:")
    print("  enable   - 메모리 분석 기능 활성화")
    print("  disable  - 메모리 분석 기능 비활성화")
    print("  compile  - 현재 설정으로 프로젝트 컴파일")
    print("  status   - 현재 메모리 분석 기능 상태 확인")
    print("  help     - 이 도움말 표시")

def check_status():
    """현재 메모리 분석 기능 상태 확인"""
    print("현재 메모리 분석 기능 상태 확인 중...")
    
    # 파일 확인
    files_exist = all(os.path.exists(os.path.join(PROJECT_DIR, file)) for file in MEMORY_FILES)
    
    # 백업 확인
    backups_exist = all(os.path.exists(os.path.join(BACKUP_DIR, f"{file}.orig")) for file in MEMORY_FILES)
    
    # exclude_memory_analysis.h 확인
    exclude_file = os.path.join(PROJECT_DIR, "exclude_memory_analysis.h")
    enabled = False
    
    if os.path.exists(exclude_file):
        with open(exclude_file, 'r', encoding='utf-8') as f:
            content = f.read()
        enabled = "#define MEMORY_ANALYSIS_ENABLED 1" in content
    
    print("\n=== 메모리 분석 기능 상태 ===")
    print(f"활성화 상태: {'활성화' if enabled else '비활성화'}")
    print(f"원본 파일: {'모두 있음' if files_exist else '일부 누락'}")
    print(f"백업 파일: {'모두 있음' if backups_exist else '일부 누락'}")
    
    # 파일 상태 자세히 보기
    print("\n파일 상태:")
    for file in MEMORY_FILES:
        source_path = os.path.join(PROJECT_DIR, file)
        backup_path = os.path.join(BACKUP_DIR, f"{file}.orig")
        
        source_exists = os.path.exists(source_path)
        backup_exists = os.path.exists(backup_path)
        
        print(f"  {file}:")
        print(f"    원본: {'있음' if source_exists else '없음'}")
        print(f"    백업: {'있음' if backup_exists else '없음'}")
        
        if source_exists:
            # 파일 크기로 빈 구현 여부 추정
            source_size = os.path.getsize(source_path)
            print(f"    크기: {source_size} 바이트 {'(빈 구현으로 보임)' if source_size < 500 else ''}")

def main():
    """메인 함수"""
    if len(sys.argv) < 2:
        show_help()
        return
    
    command = sys.argv[1].lower()
    
    if command == "enable":
        enable_memory_analysis()
    elif command == "disable":
        disable_memory_analysis()
    elif command == "compile":
        compile_project()
    elif command == "status":
        check_status()
    elif command in ["help", "-h", "--help"]:
        show_help()
    else:
        print(f"알 수 없는 명령: {command}")
        show_help()

if __name__ == "__main__":
    main()
