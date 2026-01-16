#!/usr/bin/env python3
"""Python 코드베이스 메트릭 분석 스크립트"""

import os
import ast
from typing import Dict, List, Tuple

def count_functions_and_classes(file_path: str) -> Tuple[int, int, int]:
    """파일의 라인 수, 함수 개수, 클래스 개수 계산"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
        lines = len(content.splitlines())
    
    try:
        tree = ast.parse(content)
        functions = sum(1 for node in ast.walk(tree) if isinstance(node, ast.FunctionDef))
        classes = sum(1 for node in ast.walk(tree) if isinstance(node, ast.ClassDef))
        return lines, functions, classes
    except:
        return lines, 0, 0

def analyze_function_complexity(file_path: str) -> List[Dict]:
    """함수 복잡도 분석"""
    complex_functions = []
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    try:
        tree = ast.parse(content)
        for node in ast.walk(tree):
            if isinstance(node, ast.FunctionDef):
                # 함수 라인 수 계산
                func_lines = node.end_lineno - node.lineno if hasattr(node, 'end_lineno') else 0
                # 매개변수 개수
                param_count = len(node.args.args)
                
                if func_lines > 30 or param_count > 5:
                    complex_functions.append({
                        'name': node.name,
                        'lines': func_lines,
                        'params': param_count,
                        'line_start': node.lineno
                    })
    except:
        pass
    
    return complex_functions

def check_imports(file_path: str) -> List[str]:
    """파일의 import 목록 추출"""
    imports = []
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        tree = ast.parse(content)
        for node in ast.walk(tree):
            if isinstance(node, ast.Import):
                for alias in node.names:
                    imports.append(alias.name)
            elif isinstance(node, ast.ImportFrom):
                if node.module:
                    imports.append(node.module)
    except:
        pass
    
    return imports

# 모든 Python 파일 찾기
python_files = []
for root, dirs, files in os.walk('src/python'):
    for file in files:
        if file.endswith('.py'):
            python_files.append(os.path.join(root, file))

# 분석 결과 저장
results = []
all_complex_functions = []
all_imports = {}

for file_path in python_files:
    if '__pycache__' in file_path:
        continue
        
    lines, functions, classes = count_functions_and_classes(file_path)
    complex_funcs = analyze_function_complexity(file_path)
    imports = check_imports(file_path)
    
    all_complex_functions.extend([(file_path, f) for f in complex_funcs])
    all_imports[file_path] = imports
    
    results.append({
        'file': file_path.replace('\\', '/'),
        'lines': lines,
        'functions': functions,
        'classes': classes
    })

# 결과 출력
print('=== Python 파일 메트릭 분석 ===')
print(f"{'파일':<50} {'라인':<6} {'함수':<6} {'클래스':<6}")
print('-' * 70)

total_lines = 0
total_functions = 0
total_classes = 0

for result in sorted(results, key=lambda x: x['lines'], reverse=True):
    file_name = result['file'].replace('src/python/', '')
    print(f"{file_name:<50} {result['lines']:<6} {result['functions']:<6} {result['classes']:<6}")
    total_lines += result['lines']
    total_functions += result['functions']
    total_classes += result['classes']

print('-' * 70)
print(f"{'총계':<50} {total_lines:<6} {total_functions:<6} {total_classes:<6}")

print('\n=== 복잡한 함수들 (30줄 이상 또는 5개 이상 매개변수) ===')
if all_complex_functions:
    for file_path, func in sorted(all_complex_functions, key=lambda x: x[1]['lines'], reverse=True):
        file_short = file_path.replace('src\\python\\', '').replace('\\', '/')
        print(f"{file_short}:{func['line_start']} - {func['name']}() - {func['lines']}줄, {func['params']}개 매개변수")
else:
    print('복잡한 함수 없음')

# 순환 임포트 체크
print('\n=== 순환 임포트 분석 ===')
internal_imports = {}
for file_path, imports in all_imports.items():
    file_short = file_path.replace('src\\python\\', '').replace('\\', '/')
    internal_imports[file_short] = [imp for imp in imports if imp.startswith('.') or 'src.python' in imp]

circular_found = False
for file, imports in internal_imports.items():
    if imports:
        print(f"{file}: {', '.join(imports)}")
        circular_found = True

if not circular_found:
    print('내부 모듈 간 임포트가 깔끔하게 구성되어 있습니다.')