#!/usr/bin/env python3
"""간단한 센서 로그 카드 테스트

과도한 정보 표시를 줄이고 핵심 정보만 표시하는 간단한 로그 카드를 테스트합니다.
"""

import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), 'src', 'python'))

from dash import Dash, html, dcc
from dash.dependencies import Input, Output
import time
from components.sensor_log_card import sensor_log_card, multi_sensor_log_cards

# 테스트용 센서 데이터
test_sensor_data = {
    "28FF1234": {
        'sensor_id': '01',
        'sensor_addr': '28FF1234567890AB',
        'last_temperature': 23.5,
        'average_temperature': 22.8,
        'min_temperature': 20.1,
        'max_temperature': 25.3,
        'total_measurements': 150,
        'successful_measurements': 147,
        'consecutive_successes': 12,
        'max_consecutive_successes': 25,
        'connection_failures': 3,
        'last_failure_time': None,
        'high_threshold_violations': 0,
        'low_threshold_violations': 0,
        'th_value': 30.0,
        'tl_value': 15.0,
        'temperature_change_rate': 0.2
    },
    "28FF5678": {
        'sensor_id': '02',
        'sensor_addr': '28FF567890ABCDEF',
        'last_temperature': 28.7,
        'average_temperature': 27.9,
        'min_temperature': 25.2,
        'max_temperature': 31.1,
        'total_measurements': 145,
        'successful_measurements': 140,
        'consecutive_successes': 8,
        'max_consecutive_successes': 20,
        'connection_failures': 5,
        'last_failure_time': "2분 전",
        'high_threshold_violations': 2,
        'low_threshold_violations': 0,
        'th_value': 30.0,
        'tl_value': 15.0,
        'temperature_change_rate': -0.8
    },
    "28FF9ABC": {
        'sensor_id': '03',
        'sensor_addr': '28FF9ABCDEF01234',
        'last_temperature': 19.2,
        'average_temperature': 20.5,
        'min_temperature': 18.0,
        'max_temperature': 23.8,
        'total_measurements': 120,
        'successful_measurements': 115,
        'consecutive_successes': 0,
        'max_consecutive_successes': 15,
        'connection_failures': 8,
        'last_failure_time': "방금 전",
        'high_threshold_violations': 0,
        'low_threshold_violations': 1,
        'th_value': 30.0,
        'tl_value': 15.0,
        'temperature_change_rate': 0.0
    }
}

# Dash 앱 생성
app = Dash(__name__, assets_folder='src/python/assets')

app.layout = html.Div([
    html.H1("간단한 센서 로그 카드 테스트", style={'textAlign': 'center', 'marginBottom': '30px'}),
    
    html.Div([
        html.H3("개별 센서 로그 카드 (간단 버전)"),
        html.Div([
            html.Div([
                sensor_log_card(
                    sensor_stats=test_sensor_data["28FF1234"],
                    system_start_time=time.time() - 3600,  # 1시간 전 시작
                    id_prefix="sensor-1"
                )
            ], style={'width': '300px', 'height': '100px', 'border': '1px solid #ccc', 'margin': '10px'}),
            
            html.Div([
                sensor_log_card(
                    sensor_stats=test_sensor_data["28FF5678"],
                    system_start_time=time.time() - 3600,
                    id_prefix="sensor-2"
                )
            ], style={'width': '300px', 'height': '100px', 'border': '1px solid #ccc', 'margin': '10px'}),
            
            html.Div([
                sensor_log_card(
                    sensor_stats=test_sensor_data["28FF9ABC"],
                    system_start_time=time.time() - 3600,
                    id_prefix="sensor-3"
                )
            ], style={'width': '300px', 'height': '100px', 'border': '1px solid #ccc', 'margin': '10px'}),
        ], style={'display': 'flex', 'flexWrap': 'wrap', 'justifyContent': 'center'}),
    ], style={'marginBottom': '40px'}),
    
    html.Div([
        html.H3("다중 센서 로그 카드 (간단 버전)"),
        html.Div([
            multi_sensor_log_cards(
                sensors_stats=test_sensor_data,
                system_start_time=time.time() - 3600,
                max_cards=3
            )
        ], style={'width': '400px', 'height': '300px', 'border': '1px solid #ccc', 'margin': '0 auto'}),
    ], style={'marginBottom': '40px'}),
    
    html.Div([
        html.H3("기존 vs 새로운 비교"),
        html.P("새로운 간단한 로그 카드는 다음 정보만 표시합니다:"),
        html.Ul([
            html.Li("🌡️ 현재 온도"),
            html.Li("📡 통신 성공률"),
            html.Li("✅ 연속 성공 횟수 + 알림 개수"),
        ]),
        html.P("기존의 4개 섹션(온도 정보, 통신 상태, 시스템 정보, 알림 정보)에서 핵심 정보만 1줄로 압축했습니다."),
    ], style={'textAlign': 'center', 'marginTop': '40px'}),
    
    dcc.Interval(
        id='interval-component',
        interval=2000,  # 2초마다 업데이트
        n_intervals=0
    )
], className='app-root')

@app.callback(
    Output('sensor-1-log-card', 'children'),
    Output('sensor-2-log-card', 'children'),
    Output('sensor-3-log-card', 'children'),
    Input('interval-component', 'n_intervals')
)
def update_log_cards(n):
    """로그 카드 업데이트 (실시간 시뮬레이션)"""
    import random
    
    # 온도 값을 약간씩 변경
    for addr, data in test_sensor_data.items():
        data['last_temperature'] += random.uniform(-0.5, 0.5)
        data['consecutive_successes'] = max(0, data['consecutive_successes'] + random.randint(-1, 2))
        data['total_measurements'] += 1
        if random.random() > 0.1:  # 90% 성공률
            data['successful_measurements'] += 1
    
    # 새로운 카드들 생성
    card1_children = sensor_log_card(
        sensor_stats=test_sensor_data["28FF1234"],
        system_start_time=time.time() - 3600,
        id_prefix="sensor-1"
    ).children
    
    card2_children = sensor_log_card(
        sensor_stats=test_sensor_data["28FF5678"],
        system_start_time=time.time() - 3600,
        id_prefix="sensor-2"
    ).children
    
    card3_children = sensor_log_card(
        sensor_stats=test_sensor_data["28FF9ABC"],
        system_start_time=time.time() - 3600,
        id_prefix="sensor-3"
    ).children
    
    return card1_children, card2_children, card3_children

if __name__ == '__main__':
    print("간단한 센서 로그 카드 테스트 시작...")
    print("브라우저에서 http://127.0.0.1:8050 을 열어주세요")
    print("Ctrl+C로 종료")
    
    app.run(debug=True, port=8050)