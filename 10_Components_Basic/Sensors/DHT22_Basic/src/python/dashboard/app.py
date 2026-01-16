"""
DHT22 Environmental Monitoring Dashboard
Simple Dash application for real-time sensor monitoring
"""

import os
import sys
import threading
import time

import dash
import pandas as pd
import plotly.graph_objs as go
from dash import Input, Output, dcc, html

# Add src/python directory to path
current_dir = os.path.dirname(__file__)
python_dir = os.path.dirname(current_dir)
sys.path.append(python_dir)

from utils.data_processor import DataBuffer, process_sensor_data
from utils.serial_reader import DHT22SerialReader, DHT22Simulator

# Initialize components
USE_SIMULATOR = True  # Set to False when Arduino is connected
data_buffer = DataBuffer(max_size=200)

# Initialize sensor reader
if USE_SIMULATOR:
    sensor = DHT22Simulator()
    print("Using DHT22 Simulator")
else:
    sensor = DHT22SerialReader(port="COM3")  # Adjust port as needed
    if not sensor.connect():
        print("Failed to connect to Arduino, falling back to simulator")
        sensor = DHT22Simulator()
        USE_SIMULATOR = True

# Initialize Dash app
app = dash.Dash(__name__)
app.title = "DHT22 Environmental Monitor"

# App layout
app.layout = html.Div(
    [
        html.Div(
            [
                html.H1("🌡️ DHT22 환경 모니터링", className="header-title"),
                html.P(
                    "실시간 온도, 습도 및 체감 지수 모니터링",
                    className="header-subtitle",
                ),
                html.Div(id="status-indicator", className="status-indicator"),
            ],
            className="header",
        ),
        # Current readings cards (2x2 grid, full width)
        html.Div(
            [
                html.Div(
                    [
                        # 온도 카드
                        html.Div(
                            [
                                html.Div(
                                    [
                                        html.Span("🌡️", className="metric-icon"),
                                        html.Div(
                                            id="current-temperature",
                                            className="metric-value",
                                        ),
                                        html.Span("°C", className="metric-unit"),
                                    ],
                                    className="metric-row",
                                ),
                                html.H3("온도", className="card-title"),
                            ],
                            className="metric-card",
                        ),
                        # 습도 카드
                        html.Div(
                            [
                                html.Div(
                                    [
                                        html.Span("💧", className="metric-icon"),
                                        html.Div(
                                            id="current-humidity",
                                            className="metric-value",
                                        ),
                                        html.Span("%", className="metric-unit"),
                                    ],
                                    className="metric-row",
                                ),
                                html.H3("습도", className="card-title"),
                            ],
                            className="metric-card",
                        ),
                        # 이슬점 카드
                        html.Div(
                            [
                                html.Div(
                                    [
                                        html.Span("🌫️", className="metric-icon"),
                                        html.Div(
                                            id="current-dewpoint",
                                            className="metric-value",
                                        ),
                                        html.Span("°C", className="metric-unit"),
                                    ],
                                    className="metric-row",
                                ),
                                html.H3("이슬점", className="card-title"),
                            ],
                            className="metric-card",
                        ),
                        # 체감 지수 카드
                        html.Div(
                            [
                                html.Div(
                                    [
                                        html.Div(
                                            id="current-discomfort",
                                            className="metric-value",
                                        ),
                                        html.Div(
                                            id="comfort-level",
                                            className="comfort-level-inline",
                                        ),
                                    ],
                                    className="metric-row",
                                ),
                                html.H3("체감 지수", className="card-title"),
                            ],
                            className="metric-card",
                        ),
                    ],
                    className="metrics-grid-2x2",
                ),
            ],
            className="metrics-full",
        ),
        # Charts
        html.Div(
            [
                html.H2("📊 실시간 차트"),
                dcc.Graph(id="temperature-chart"),
                dcc.Graph(id="humidity-chart"),
            ],
            className="charts-section",
        ),
        # Statistics
        html.Div(
            [html.H2("📈 통계"), html.Div(id="statistics-table")],
            className="stats-section",
        ),
        # Auto-refresh interval
        dcc.Interval(
            id="interval-component",
            interval=2000,  # Update every 2 seconds
            n_intervals=0,
        ),
    ],
    className="container",
)


# Data collection thread
def data_collection_thread():
    """Background thread for collecting sensor data"""
    while True:
        try:
            raw_data = sensor.read_sensor_data()
            if raw_data:
                processed_data = process_sensor_data(raw_data)
                data_buffer.add(processed_data)
        except Exception as e:
            print(f"Error collecting data: {e}")
        time.sleep(2)


# Start data collection
threading.Thread(target=data_collection_thread, daemon=True).start()


# Callbacks
@app.callback(
    [
        Output("current-temperature", "children"),
        Output("current-humidity", "children"),
        Output("current-dewpoint", "children"),
        Output("current-discomfort", "children"),
        Output("comfort-level", "children"),
        Output("status-indicator", "children"),
    ],
    [Input("interval-component", "n_intervals")],
)
def update_current_values(n):
    """Update current sensor readings"""
    recent_data = data_buffer.get_recent(1)

    if not recent_data:
        return "—", "—", "—", "—", "데이터 없음", "🔴 연결 안됨"

    data = recent_data[0]

    status = "🟢 연결됨 (시뮬)" if USE_SIMULATOR else "🟢 연결됨"

    return (
        f"{data['temperature']:.1f}",
        f"{data['humidity']:.1f}",
        f"{data['dew_point']:.1f}",
        f"{data['discomfort_index']:.1f}",
        data["comfort_level"],
        status,
    )


@app.callback(
    Output("temperature-chart", "figure"), [Input("interval-component", "n_intervals")]
)
def update_temperature_chart(n):
    """Update temperature chart"""
    recent_data = data_buffer.get_recent(50)  # Last 50 readings

    if not recent_data:
        return go.Figure()

    df = pd.DataFrame(recent_data)
    df["time"] = pd.to_datetime(df["python_timestamp"], unit="s")

    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["time"],
            y=df["temperature"],
            mode="lines+markers",
            name="온도",
            line={"color": "#ff6b6b", "width": 2},
        )
    )

    fig.update_layout(
        title="온도 추이",
        xaxis_title="시간",
        yaxis_title="온도 (°C)",
        height=300,
        margin={"l": 50, "r": 50, "t": 50, "b": 50},
    )

    return fig


@app.callback(
    Output("humidity-chart", "figure"), [Input("interval-component", "n_intervals")]
)
def update_humidity_chart(n):
    """Update humidity chart"""
    recent_data = data_buffer.get_recent(50)  # Last 50 readings

    if not recent_data:
        return go.Figure()

    df = pd.DataFrame(recent_data)
    df["time"] = pd.to_datetime(df["python_timestamp"], unit="s")

    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["time"],
            y=df["humidity"],
            mode="lines+markers",
            name="습도",
            line={"color": "#4ecdc4", "width": 2},
        )
    )

    fig.update_layout(
        title="습도 추이",
        xaxis_title="시간",
        yaxis_title="습도 (%)",
        height=300,
        margin={"l": 50, "r": 50, "t": 50, "b": 50},
    )

    return fig


@app.callback(
    Output("statistics-table", "children"), [Input("interval-component", "n_intervals")]
)
def update_statistics(n):
    """Update statistics table"""
    stats = data_buffer.get_stats()

    if not stats:
        return html.P("통계 데이터가 없습니다.")

    table_rows = []
    for metric, values in stats.items():
        if metric in ["temperature", "humidity", "dew_point", "discomfort_index"]:
            korean_names = {
                "temperature": "온도 (°C)",
                "humidity": "습도 (%)",
                "dew_point": "이슬점 (°C)",
                "discomfort_index": "불쾌지수",
            }

            table_rows.append(
                html.Tr(
                    [
                        html.Td(korean_names[metric]),
                        html.Td(f"{values['min']:.1f}"),
                        html.Td(f"{values['max']:.1f}"),
                        html.Td(f"{values['mean']:.1f}"),
                        html.Td(f"{values['current']:.1f}"),
                    ]
                )
            )

    return html.Table(
        [
            html.Thead(
                [
                    html.Tr(
                        [
                            html.Th("측정값"),
                            html.Th("최소값"),
                            html.Th("최대값"),
                            html.Th("평균값"),
                            html.Th("현재값"),
                        ]
                    )
                ]
            ),
            html.Tbody(table_rows),
        ],
        className="stats-table",
    )


# CSS styling
app.index_string = """
<!DOCTYPE html>
<html>
    <head>
        {%metas%}
        <title>{%title%}</title>
        {%favicon%}
        {%css%}
        <style>
            /* 전체 페이지 배경 및 폰트 설정 */
            body {
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                margin: 0;
                padding: 0;
                background-color: #f5f7fa;
            }
            /* 대시보드 전체 컨테이너 */
            .container {
                max-width: 1200px;
                margin: 0 auto;
                padding: 20px;
            }
            /* 상단 헤더 영역 */
            .header {
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                color: white;
                padding: 30px;
                border-radius: 12px;
                margin-bottom: 30px;
                text-align: center;
            }
            /* 헤더 타이틀 */
            .header-title {
                margin: 0 0 10px 0;
                font-size: 2.5em;
            }
            /* 헤더 서브타이틀 */
            .header-subtitle {
                margin: 0;
                opacity: 0.9;
                font-size: 1.2em;
            }
            /* 연결 상태 표시 */
            .status-indicator {
                margin-top: 15px;
                font-size: 1.1em;
                font-weight: bold;
            }
            /* 측정값 카드 전체 영역 (가운데 정렬) */
            .metrics-full {
                width: 100%;
                display: flex;
                justify-content: center;
                align-items: center;
                margin-bottom: 30px;
            }
            /* 2x2 측정값 카드 그리드 (온도/습도/이슬점/체감지수) */
            .metrics-grid-2x2 {
                display: grid;
                grid-template-columns: 1fr 1fr;
                grid-template-rows: 1fr 1fr;
                column-gap: 130px;
                row-gap: 30px;
                width: 100%;
                max-width: 600px;
            }
            /* 개별 측정값 카드 스타일 */
            .metric-card {
                background: white;
                padding: 32px 0 32px 0;
                border-radius: 12px;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                text-align: center;
                width: 100%;
                min-width: 0;
                min-height: 160px;
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
            }
            /* 값+아이콘+단위 한 줄 배치 */
            .metric-row {
                display: flex;
                flex-direction: row;
                align-items: center;
                justify-content: center;
                gap: 10px;
                margin-bottom: 8px;
            }
            .metric-icon {
                font-size: 2em;
                margin-left: 2px;
                margin-right: 2px;
            }
            .comfort-level-inline {
                margin-left: 8px;
                padding: 6px 14px;
                border-radius: 20px;
                font-weight: bold;
                color: white;
                background-color: #3498db;
                font-size: 1em;
                display: inline-block;
            }
            /* 개별 측정값 카드 스타일 (중복 정의, 필요시 하나만 유지) */
            .metric-card {
                background: white;
                padding: 25px;
                border-radius: 12px;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                text-align: center;
            }
            /* 카드 타이틀 (예: 온도, 습도 등) */
            .card-title {
                margin: 0 0 15px 0;
                color: #333;
                font-size: 1.2em;
            }
            /* 카드 내 값 표시 (숫자) */
            .metric-value {
                font-size: 2.5em;
                font-weight: bold;
                color: #2c3e50;
                margin-bottom: 5px;
            }
            /* 단위 표시 (°C, %) */
            .metric-unit {
                color: #7f8c8d;
                font-size: 2.5em;
            }
            /* 체감 지수 레벨 표시 (쾌적 등) */
            .comfort-level {
                margin-top: 10px;
                padding: 8px 16px;
                border-radius: 20px;
                font-weight: bold;
                color: white;
                background-color: #3498db;
            }
            /* 차트/통계 영역 카드 스타일 */
            .charts-section, .stats-section {
                background: white;
                padding: 25px;
                border-radius: 12px;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                margin-bottom: 30px;
            }
            /* 통계 테이블 전체 */
            .stats-table {
                width: 100%;
                border-collapse: collapse;
                margin-top: 20px;
            }
            /* 통계 테이블 셀 스타일 */
            .stats-table th, .stats-table td {
                padding: 12px;
                text-align: left;
                border-bottom: 1px solid #ddd;
            }
            /* 통계 테이블 헤더 셀 스타일 */
            .stats-table th {
                background-color: #f8f9fa;
                font-weight: bold;
            }
        </style>
    </head>
    <body>
        {%app_entry%}
        <footer>
            {%config%}
            {%scripts%}
            {%renderer%}
        </footer>
    </body>
</html>
"""

if __name__ == "__main__":
    print("Starting DHT22 Environmental Monitor...")
    print(f"Using {'Simulator' if USE_SIMULATOR else 'Arduino'} for data")
    print("Dashboard available at: http://localhost:8050")

    app.run(debug=False, host="0.0.0.0", port=8050)
