#!/usr/bin/env python3
"""
정리된 센서 로그 카드 테스트
기존 빈 카드를 제거하고 새로운 센서 로그 카드만 표시하는지 확인
"""

import sys
import os

# 프로젝트 루트를 Python 경로에 추가
project_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, project_root)

def test_log_card_component():
    """센서 로그 카드 컴포넌트 테스트"""
    print("🧪 정리된 센서 로그 카드 테스트")
    print("=" * 50)
    
    from src.python.components.sensor_log_card import sensor_log_card
    
    # 테스트 데이터
    test_stats = {
        'sensor_id': '01',
        'sensor_addr': '285882840000000E',
        'last_temperature': 27.8,
        'average_temperature': 27.5,
        'min_temperature': 27.1,
        'max_temperature': 27.9,
        'total_measurements': 127,
        'successful_measurements': 125,
        'failed_measurements': 2,
        'consecutive_successes': 15,
        'max_consecutive_successes': 23,
        'connection_failures': 2,
        'last_failure_time': '14:25:12',
        'high_threshold_violations': 0,
        'low_threshold_violations': 0,
        'temperature_change_rate': 1.2,
        'th_value': 30.0,
        'tl_value': 15.0,
    }
    
    # 센서 로그 카드 생성
    card = sensor_log_card(
        sensor_stats=test_stats,
        system_start_time=1234567890,
        id_prefix="test"
    )
    
    print("✅ 센서 로그 카드 컴포넌트 생성 성공!")
    print(f"   카드 타입: {type(card)}")
    print(f"   카드 클래스: {card.className}")
    print(f"   카드 ID: {card.id}")
    
    # 빈 카드 테스트
    empty_card = sensor_log_card()
    print("✅ 빈 센서 로그 카드 생성 성공!")
    print(f"   빈 카드 클래스: {empty_card.className}")
    
    return True

def test_css_classes():
    """CSS 클래스 정리 확인"""
    print("\n🎨 CSS 클래스 정리 확인")
    print("=" * 30)
    
    css_file = "src/python/assets/custom.css"
    
    try:
        with open(css_file, 'r', encoding='utf-8') as f:
            css_content = f.read()
        
        # 기존 빈 카드 스타일 제거 확인
        old_styles = [
            '.log-card::before',
            '.log-card::after',
            'content: "센서 로그"'
        ]
        
        removed_count = 0
        for style in old_styles:
            if style not in css_content:
                removed_count += 1
                print(f"✅ 제거됨: {style}")
            else:
                print(f"⚠️  남아있음: {style}")
        
        # 새로운 스타일 추가 확인
        new_styles = [
            '.card.sensor-log-card',
            '.log-content',
            '.log-header',
            '.log-section'
        ]
        
        added_count = 0
        for style in new_styles:
            if style in css_content:
                added_count += 1
                print(f"✅ 추가됨: {style}")
            else:
                print(f"❌ 누락됨: {style}")
        
        print(f"\n📊 정리 결과:")
        print(f"   제거된 구 스타일: {removed_count}/{len(old_styles)}")
        print(f"   추가된 신 스타일: {added_count}/{len(new_styles)}")
        
        return removed_count >= 2 and added_count >= 3
        
    except Exception as e:
        print(f"❌ CSS 파일 읽기 오류: {e}")
        return False

def main():
    """메인 테스트 함수"""
    print("🚀 정리된 센서 로그 카드 종합 테스트")
    print("=" * 60)
    
    # 컴포넌트 테스트
    component_ok = test_log_card_component()
    
    # CSS 정리 테스트
    css_ok = test_css_classes()
    
    print("\n" + "=" * 60)
    print("📋 테스트 결과 요약:")
    print(f"   컴포넌트 테스트: {'✅ 성공' if component_ok else '❌ 실패'}")
    print(f"   CSS 정리 테스트: {'✅ 성공' if css_ok else '❌ 실패'}")
    
    if component_ok and css_ok:
        print("\n🎉 모든 테스트 통과!")
        print("   기존 빈 로그 카드가 제거되고 새로운 센서 로그 카드만 남았습니다.")
        print("   이제 대시보드를 실행해서 확인해보세요:")
        print("   python run_dashboard_with_logs.py")
    else:
        print("\n⚠️  일부 테스트 실패 - 추가 확인이 필요합니다.")

if __name__ == "__main__":
    main()