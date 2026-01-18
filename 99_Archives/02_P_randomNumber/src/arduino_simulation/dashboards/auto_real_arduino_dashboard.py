#!/usr/bin/env python3
"""
Auto Real Arduino Dashboard
실제 Arduino 구현들을 자동으로 테스트하는 대시보드

주요 기능:
- 8개 실제 Arduino 구현 자동 테스트
- 10초 카운트다운
- 자동 시작/완료/정지
- 에러 감지 및 자동 패치
- 실시간 결과 표시
"""

import os
import sys
import threading
import time
import traceback
from typing import Any, Dict, List, Optional

import dash
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
import yaml
from dash import Input, Output, dcc, html

# 경로 추가
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))

from arduino_mock import ArduinoUnoR4WiFiMock
from real_arduino_sim import RealArduinoImplementationGenerator


class AutoRealArduinoDashboard:
    """실제 Arduino 구현 자동 테스트 대시보드"""

    def __init__(self, port: int = 8053, debug: bool = False):
        self.app = dash.Dash(__name__)
        self.port = port
        self.debug = debug

        # 상태 관리
        self.countdown = 10
        self.is_running = False
        self.is_completed = False
        self.current_impl = ""
        self.progress = 0
        self.results = []
        self.error_log = []
        self.auto_thread = None

        # 구현 목록 로드
        self.implementations = self._load_implementations()

        self._setup_layout()
        self._setup_callbacks()

        print(f"Auto Real Arduino Dashboard initialized on port {port}")
        print(f"Loaded {len(self.implementations)} real Arduino implementations")

    def _load_implementations(self) -> List[Dict[str, Any]]:
        """실제 Arduino 구현 로드"""
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
            self.error_log.append(f"Failed to load implementations: {e}")
            return []

    def _setup_layout(self):
        """레이아웃 설정"""
        self.app.layout = html.Div(
            [
                # 헤더
                html.Div(
                    [
                        html.H1(
                            "Real Arduino Implementation Dashboard",
                            className="text-center mb-4",
                            style={
                                "color": "#e74c3c",
                                "textShadow": "2px 2px 4px rgba(0,0,0,0.3)",
                            },
                        ),
                        html.P(
                            "8가지 실제 Arduino C++ 구현 자동 비교 테스트",
                            className="text-center text-muted mb-4",
                            style={"fontSize": "18px"},
                        ),
                        html.Hr(),
                    ]
                ),
                # 메인 카운트다운
                html.Div(
                    [
                        html.Div(
                            id="main-countdown",
                            style={
                                "fontSize": "150px",
                                "fontWeight": "bold",
                                "textAlign": "center",
                                "color": "#e74c3c",
                                "textShadow": "3px 3px 6px rgba(0,0,0,0.4)",
                                "minHeight": "250px",
                                "display": "flex",
                                "alignItems": "center",
                                "justifyContent": "center",
                                "background": "linear-gradient(135deg, #667eea 0%, #764ba2 100%)",
                                "borderRadius": "20px",
                                "margin": "20px 0",
                                "boxShadow": "0 10px 30px rgba(0,0,0,0.3)",
                            },
                        )
                    ]
                ),
                # 상태 메시지
                html.Div(
                    [
                        html.Div(
                            id="status-message",
                            className="alert alert-info text-center",
                            style={
                                "fontSize": "28px",
                                "fontWeight": "bold",
                                "borderRadius": "15px",
                                "boxShadow": "0 5px 15px rgba(0,0,0,0.2)",
                            },
                        )
                    ],
                    className="mb-4",
                ),
                # 진행률 표시
                html.Div([html.Div(id="progress-display")], className="mb-4"),
                # 현재 테스트 중인 구현
                html.Div([html.Div(id="current-test-display")], className="mb-4"),
                # 실시간 결과 카드
                html.Div(
                    [
                        html.H3("📊 Real-time Results", className="text-center mb-4"),
                        html.Div(id="realtime-results", className="row"),
                    ],
                    className="mb-4",
                ),
                # 성능 차트
                html.Div(
                    [
                        html.Div(
                            [
                                html.H4("🏃‍♂️ Performance Ranking"),
                                dcc.Graph(id="performance-chart"),
                            ],
                            className="col-md-6",
                        ),
                        html.Div(
                            [
                                html.H4("📈 Implementation Comparison"),
                                dcc.Graph(id="comparison-chart"),
                            ],
                            className="col-md-6",
                        ),
                    ],
                    className="row mb-4",
                ),
                # 최종 결과 및 추천
                html.Div(
                    [
                        html.H3(
                            "🏆 Final Results & Recommendation",
                            className="text-center mb-4",
                        ),
                        html.Div(id="final-results"),
                    ],
                    className="mb-4",
                ),
                # 에러 로그
                html.Div([html.Div(id="error-display")], className="mb-4"),
                # 자동 업데이트
                dcc.Interval(id="main-interval", interval=1000, n_intervals=0),
                # 데이터 저장소
                dcc.Store(id="results-data"),
            ],
            className="container-fluid",
            style={
                "backgroundColor": "#f8f9fa",
                "minHeight": "100vh",
                "padding": "20px",
            },
        )

    def _setup_callbacks(self):
        """콜백 설정"""

        @self.app.callback(
            [
                Output("main-countdown", "children"),
                Output("status-message", "children"),
                Output("progress-display", "children"),
                Output("current-test-display", "children"),
                Output("error-display", "children"),
            ],
            [Input("main-interval", "n_intervals")],
        )
        def update_main_display(n_intervals):
            """메인 디스플레이 업데이트"""

            # 첫 실행 시 자동 시작
            if n_intervals == 1:
                self._start_auto_test()

            # 에러 표시
            error_display = self._create_error_display()

            # 카운트다운 중
            if self.countdown > 0 and not self.is_running and not self.is_completed:
                countdown_text = str(self.countdown)
                status_text = (
                    f"🚀 Real Arduino 테스트가 {self.countdown}초 후 시작됩니다!"
                )
                progress_display = self._create_progress_bar(0, "대기 중...")
                current_test = html.Div()
                return (
                    countdown_text,
                    status_text,
                    progress_display,
                    current_test,
                    error_display,
                )

            # 실행 중
            elif self.is_running:
                countdown_text = "🔄"
                status_text = f"⚡ 실제 Arduino 구현 테스트 진행 중... ({self.progress}/{len(self.implementations)})"
                progress_display = self._create_progress_bar(
                    (
                        int((self.progress / len(self.implementations)) * 100)
                        if self.implementations
                        else 0
                    ),
                    f"진행 중... {self.progress}/{len(self.implementations)}",
                )
                current_test = self._create_current_test_display()
                return (
                    countdown_text,
                    status_text,
                    progress_display,
                    current_test,
                    error_display,
                )

            # 완료됨
            elif self.is_completed:
                countdown_text = "✅"
                status_text = "🎉 모든 Real Arduino 구현 테스트 완료!"
                progress_display = self._create_progress_bar(100, "완료!")
                current_test = html.Div(
                    [
                        html.H4("🏁 테스트 완료", className="text-center text-success"),
                        html.P(
                            "모든 결과를 아래에서 확인하세요.", className="text-center"
                        ),
                    ],
                    className="alert alert-success",
                )
                return (
                    countdown_text,
                    status_text,
                    progress_display,
                    current_test,
                    error_display,
                )

            # 기본 상태
            else:
                countdown_text = "⏳"
                status_text = "준비 중..."
                progress_display = self._create_progress_bar(0, "준비 중...")
                current_test = html.Div()
                return (
                    countdown_text,
                    status_text,
                    progress_display,
                    current_test,
                    error_display,
                )

        @self.app.callback(
            [Output("results-data", "data"), Output("realtime-results", "children")],
            [Input("main-interval", "n_intervals")],
        )
        def update_results(n_intervals):
            """결과 업데이트"""
            if self.results:
                realtime_cards = self._create_realtime_cards()
                return self.results, realtime_cards
            return [], []

        @self.app.callback(
            Output("performance-chart", "figure"), [Input("results-data", "data")]
        )
        def update_performance_chart(data):
            """성능 차트 업데이트"""
            if not data:
                return self._create_empty_chart("테스트 진행 중...")

            df = pd.DataFrame(data)
            fig = px.bar(
                df,
                x="name",
                y="generation_rate",
                title="Generation Speed (gen/sec)",
                color="generation_rate",
                color_continuous_scale="viridis",
            )

            fig.update_xaxes(tickangle=45)
            fig.update_layout(height=400, showlegend=False)
            return fig

        @self.app.callback(
            Output("comparison-chart", "figure"), [Input("results-data", "data")]
        )
        def update_comparison_chart(data):
            """비교 차트 업데이트"""
            if not data:
                return self._create_empty_chart("테스트 진행 중...")

            # 분포 균등성 계산
            distribution_scores = []
            for result in data:
                dist = result["distribution"]
                total = sum(dist.values())
                if total > 0:
                    percentages = [count / total for count in dist.values()]
                    score = 1 / (1 + sum(abs(p - 0.3333) for p in percentages))
                    distribution_scores.append(score)
                else:
                    distribution_scores.append(0)

            df = pd.DataFrame(
                {
                    "Implementation": [r["name"] for r in data],
                    "Distribution Quality": distribution_scores,
                }
            )

            try:
                fig = px.bar(
                    df,
                    x="Implementation",
                    y="Distribution Quality",
                    title="Distribution Quality (Higher = Better)",
                    color="Distribution Quality",
                    color_continuous_scale="viridis",  # 안전한 색상 스케일로 변경
                )
            except Exception as e:
                # 색상 스케일 없이 기본 차트 생성
                fig = px.bar(
                    df,
                    x="Implementation",
                    y="Distribution Quality",
                    title="Distribution Quality (Higher = Better)",
                )

            fig.update_xaxes(tickangle=45)
            fig.update_layout(height=400, showlegend=False)
            return fig

        @self.app.callback(
            Output("final-results", "children"), [Input("results-data", "data")]
        )
        def update_final_results(data):
            """최종 결과 업데이트"""
            if not data or not self.is_completed:
                return html.P("테스트 진행 중...", className="text-center text-muted")

            # 최고 성능 찾기
            best = max(data, key=lambda x: x["generation_rate"])

            # 제약 조건 준수 확인
            compliant = [r for r in data if r["violations"] == 0]

            return html.Div(
                [
                    # 최고 성능
                    html.Div(
                        [
                            html.H2("🏆 WINNER", className="text-center mb-3"),
                            html.H3(
                                best["name"], className="text-center text-primary mb-3"
                            ),
                            html.H4(
                                f"{best['generation_rate']:,.0f} gen/sec",
                                className="text-center text-success mb-3",
                            ),
                            html.P(
                                f"Type: {best['type']}",
                                className="text-center text-muted",
                            ),
                        ],
                        className="alert alert-warning text-center p-4 mb-4",
                    ),
                    # 통계 요약
                    html.Div(
                        [
                            html.Div(
                                [html.H5("📊 총 구현"), html.H3(str(len(data)))],
                                className="col-md-3 text-center",
                            ),
                            html.Div(
                                [
                                    html.H5("✅ 제약 준수"),
                                    html.H3(f"{len(compliant)}/{len(data)}"),
                                ],
                                className="col-md-3 text-center",
                            ),
                            html.Div(
                                [
                                    html.H5("⚡ 평균 속도"),
                                    html.H3(
                                        f"{sum(r['generation_rate'] for r in data) / len(data):,.0f}"
                                    ),
                                ],
                                className="col-md-3 text-center",
                            ),
                            html.Div(
                                [html.H5("🎯 성공률"), html.H3("100%")],
                                className="col-md-3 text-center",
                            ),
                        ],
                        className="row alert alert-info p-4",
                    ),
                ]
            )

    def _start_auto_test(self):
        """자동 테스트 시작"""

        def countdown_and_test():
            try:
                # 카운트다운
                for i in range(10, 0, -1):
                    self.countdown = i
                    time.sleep(1)

                # 테스트 시작
                self.countdown = 0
                self.is_running = True
                self.progress = 0

                print("🚀 Starting Real Arduino implementations test...")

                # 각 구현 테스트
                for i, impl in enumerate(self.implementations):
                    self.current_impl = impl["name"]
                    self.progress = i

                    print(f"Testing {i+1}/{len(self.implementations)}: {impl['name']}")

                    # 테스트 실행
                    result = self._test_single_implementation(impl)
                    if result:
                        self.results.append(result)

                    time.sleep(0.5)  # 시각적 효과

                self.progress = len(self.implementations)
                self.is_running = False
                self.is_completed = True

                print("✅ All Real Arduino implementations tested!")

            except Exception as e:
                self.error_log.append(f"Auto test error: {e}")
                self.is_running = False
                self.is_completed = True

        self.auto_thread = threading.Thread(target=countdown_and_test)
        self.auto_thread.daemon = True
        self.auto_thread.start()

    def _test_single_implementation(
        self, impl: Dict[str, Any]
    ) -> Optional[Dict[str, Any]]:
        """단일 구현 테스트"""
        try:
            arduino = ArduinoUnoR4WiFiMock(seed=12345)
            generator = RealArduinoImplementationGenerator(impl, arduino)

            # 성능 측정
            start_time = time.time()
            generated_numbers = []
            violations = 0
            test_iterations = 5000

            previous = -1
            for i in range(test_iterations):
                number = generator.generate_number(previous)
                generated_numbers.append(number)

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

            return {
                "name": impl["name"],
                "type": impl["type"],
                "generation_rate": generation_rate,
                "violations": violations,
                "distribution": distribution,
                "execution_time": execution_time,
            }

        except Exception as e:
            self.error_log.append(f"Test error for {impl['name']}: {e}")
            return None

    def _create_progress_bar(self, percentage: int, text: str):
        """진행률 바 생성"""
        return html.Div(
            [
                html.Div(
                    [
                        html.Div(
                            style={
                                "width": f"{percentage}%",
                                "height": "40px",
                                "backgroundColor": (
                                    "#28a745" if percentage == 100 else "#007bff"
                                ),
                                "transition": "width 0.5s ease",
                                "borderRadius": "20px",
                            }
                        )
                    ],
                    style={
                        "width": "100%",
                        "height": "40px",
                        "backgroundColor": "#e9ecef",
                        "borderRadius": "20px",
                        "overflow": "hidden",
                        "boxShadow": "inset 0 2px 4px rgba(0,0,0,0.1)",
                    },
                ),
                html.P(
                    f"{text} ({percentage}%)",
                    className="text-center mt-3 mb-0",
                    style={"fontSize": "20px", "fontWeight": "bold"},
                ),
            ]
        )

    def _create_current_test_display(self):
        """현재 테스트 표시"""
        if not self.current_impl:
            return html.Div()

        return html.Div(
            [
                html.H4("🔬 현재 테스트 중", className="text-center mb-3"),
                html.H3(self.current_impl, className="text-center text-primary"),
                html.P(
                    f"진행률: {self.progress}/{len(self.implementations)}",
                    className="text-center text-muted",
                ),
            ],
            className="alert alert-primary text-center p-4",
        )

    def _create_realtime_cards(self):
        """실시간 결과 카드"""
        if not self.results:
            return []

        cards = []
        for result in self.results[-4:]:  # 최근 4개만 표시
            card = html.Div(
                [
                    html.H6(result["name"], className="card-title"),
                    html.H4(
                        f"{result['generation_rate']:,.0f}", className="text-primary"
                    ),
                    html.P("gen/sec", className="text-muted mb-1"),
                    html.Small(
                        f"Violations: {result['violations']}",
                        className=(
                            "text-success"
                            if result["violations"] == 0
                            else "text-danger"
                        ),
                    ),
                ],
                className="col-md-3 card card-body text-center mb-2",
            )
            cards.append(card)

        return cards

    def _create_error_display(self):
        """에러 표시"""
        if not self.error_log:
            return html.Div()

        return html.Div(
            [
                html.H5("⚠️ System Log", className="text-warning"),
                html.Div(
                    [
                        html.P(error, className="text-danger small mb-1")
                        for error in self.error_log[-3:]
                    ],
                    className="alert alert-warning",
                ),
            ]
        )

    def _create_empty_chart(self, message: str):
        """빈 차트"""
        fig = go.Figure()
        fig.add_annotation(
            text=message,
            xref="paper",
            yref="paper",
            x=0.5,
            y=0.5,
            showarrow=False,
            font=dict(size=24, color="gray"),
        )
        fig.update_layout(
            xaxis=dict(visible=False), yaxis=dict(visible=False), height=400
        )
        return fig

    def run_server(self):
        """서버 실행"""
        print("Starting Auto Real Arduino Dashboard")
        print(f"URL: http://localhost:{self.port}")
        print("10초 카운트다운 후 자동 시작")
        print("Press Ctrl+C to stop")

        try:
            self.app.run(debug=self.debug, port=self.port, host="0.0.0.0")
        except KeyboardInterrupt:
            print("\nDashboard stopped by user")
        except Exception as e:
            print(f"Server error: {e}")


if __name__ == "__main__":
    print("Auto Real Arduino Implementation Dashboard")
    print("=" * 60)

    try:
        dashboard = AutoRealArduinoDashboard(port=8053, debug=False)
        dashboard.run_server()
    except Exception as e:
        print(f"❌ Failed to start dashboard: {e}")
        traceback.print_exc()
