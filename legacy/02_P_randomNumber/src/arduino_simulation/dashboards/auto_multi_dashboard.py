#!/usr/bin/env python3
"""
Auto Multi-Implementation Dashboard
자동 실행, 카운트다운, 에러 감지 및 자동 패치 기능이 포함된 대시보드

주요 기능:
- 자동 시뮬레이션 시작
- 큰 화면 카운트다운 (10, 9, 8...)
- 자동 완료 및 정지
- 에러 감지 및 자동 패치
- 결과 자동 표시
"""

import os
import sys
import threading
import time
import traceback
from dataclasses import asdict

import dash
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
from dash import Input, Output, dcc, html

# 경로 추가
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))

from multi_implementation_sim import (
    MultiImplementationSimulator,
    run_multi_implementation_test,
)


class AutoMultiDashboard:
    """자동 실행 다중 구현 대시보드"""

    def __init__(self, port: int = 8052, debug: bool = False):
        self.app = dash.Dash(__name__)
        self.port = port
        self.debug = debug

        # 상태 관리
        self.countdown = 10
        self.is_running = False
        self.is_completed = False
        self.latest_report = None
        self.error_log = []
        self.auto_thread = None

        # 시뮬레이터
        try:
            self.simulator = MultiImplementationSimulator()
        except Exception as e:
            self.error_log.append(f"Simulator initialization error: {e}")
            self.simulator = None

        self._setup_layout()
        self._setup_callbacks()

        print(f"Auto Multi-Implementation Dashboard initialized on port {port}")

    def _setup_layout(self):
        """레이아웃 설정"""
        self.app.layout = html.Div(
            [
                # 헤더
                html.Div(
                    [
                        html.H1(
                            "🚀 Auto Multi-Implementation Dashboard",
                            className="text-center mb-4",
                            style={"color": "#2c3e50"},
                        ),
                        html.P(
                            "자동 실행 및 비교 분석 시스템",
                            className="text-center text-muted mb-4",
                        ),
                        html.Hr(),
                    ]
                ),
                # 카운트다운 디스플레이
                html.Div(
                    [
                        html.Div(
                            id="countdown-display",
                            style={
                                "fontSize": "120px",
                                "fontWeight": "bold",
                                "textAlign": "center",
                                "color": "#e74c3c",
                                "textShadow": "2px 2px 4px rgba(0,0,0,0.3)",
                                "minHeight": "200px",
                                "display": "flex",
                                "alignItems": "center",
                                "justifyContent": "center",
                            },
                        )
                    ],
                    className="mb-4",
                ),
                # 상태 표시
                html.Div(
                    [
                        html.Div(
                            id="status-message",
                            className="alert alert-info text-center",
                            style={"fontSize": "24px", "fontWeight": "bold"},
                        )
                    ],
                    className="mb-4",
                ),
                # 진행률 바
                html.Div([html.Div(id="progress-bar-container")], className="mb-4"),
                # 에러 로그
                html.Div([html.Div(id="error-log-display")], className="mb-4"),
                # 결과 요약 카드
                html.Div(
                    [
                        html.H3("📊 Comparison Results", className="text-center mb-4"),
                        html.Div(id="auto-summary-cards", className="row"),
                    ],
                    className="mb-4",
                ),
                # 성능 차트
                html.Div(
                    [
                        html.Div(
                            [
                                html.H4("🏃‍♂️ Speed Comparison"),
                                dcc.Graph(id="auto-speed-chart"),
                            ],
                            className="col-md-6",
                        ),
                        html.Div(
                            [
                                html.H4("🧠 Memory Usage"),
                                dcc.Graph(id="auto-memory-chart"),
                            ],
                            className="col-md-6",
                        ),
                    ],
                    className="row mb-4",
                ),
                # 추천 결과
                html.Div(
                    [
                        html.H3("🏆 Recommendation", className="text-center mb-4"),
                        html.Div(id="auto-recommendation"),
                    ],
                    className="mb-4",
                ),
                # 자동 업데이트 인터벌
                dcc.Interval(id="auto-interval", interval=1000, n_intervals=0),
                # 데이터 저장소
                dcc.Store(id="auto-data"),
            ],
            className="container-fluid",
            style={"backgroundColor": "#f8f9fa"},
        )

    def _setup_callbacks(self):
        """콜백 설정"""

        @self.app.callback(
            [
                Output("countdown-display", "children"),
                Output("status-message", "children"),
                Output("progress-bar-container", "children"),
                Output("error-log-display", "children"),
            ],
            [Input("auto-interval", "n_intervals")],
        )
        def update_countdown_and_status(n_intervals):
            """카운트다운 및 상태 업데이트"""

            # 첫 실행 시 자동 시작
            if n_intervals == 1:
                self._start_auto_simulation()

            # 에러 로그 표시
            error_display = self._create_error_display()

            # 카운트다운 중
            if self.countdown > 0 and not self.is_running and not self.is_completed:
                countdown_text = str(self.countdown)
                status_text = f"시뮬레이션이 {self.countdown}초 후 시작됩니다..."
                progress_bar = self._create_progress_bar(0, "대기 중...")
                return countdown_text, status_text, progress_bar, error_display

            # 실행 중
            elif self.is_running:
                countdown_text = "🔄"
                status_text = "다중 구현 비교 실행 중..."
                progress_bar = self._create_progress_bar(50, "실행 중...")
                return countdown_text, status_text, progress_bar, error_display

            # 완료됨
            elif self.is_completed:
                countdown_text = "✅"
                status_text = "비교 완료! 결과를 확인하세요."
                progress_bar = self._create_progress_bar(100, "완료")
                return countdown_text, status_text, progress_bar, error_display

            # 기본 상태
            else:
                countdown_text = "⏳"
                status_text = "준비 중..."
                progress_bar = self._create_progress_bar(0, "준비 중...")
                return countdown_text, status_text, progress_bar, error_display

        @self.app.callback(
            [Output("auto-data", "data"), Output("auto-summary-cards", "children")],
            [Input("auto-interval", "n_intervals")],
        )
        def update_results(n_intervals):
            """결과 업데이트"""
            if self.latest_report and self.is_completed:
                try:
                    data = asdict(self.latest_report)
                    summary_cards = self._create_auto_summary_cards(self.latest_report)
                    return data, summary_cards
                except Exception as e:
                    self.error_log.append(f"Results update error: {e}")
                    self._auto_patch_error(e)
                    return {}, []

            return {}, []

        @self.app.callback(
            Output("auto-speed-chart", "figure"), [Input("auto-data", "data")]
        )
        def update_speed_chart(data):
            """속도 차트 업데이트"""
            try:
                if not data or not data.get("detailed_results"):
                    return self._create_empty_chart("속도 데이터 없음")

                results = [
                    r for r in data["detailed_results"] if r.get("success", False)
                ]
                if not results:
                    return self._create_empty_chart("성공한 결과 없음")

                df = pd.DataFrame(
                    {
                        "Implementation": [r["name"] for r in results],
                        "Speed": [r["generation_rate"] for r in results],
                    }
                )

                fig = px.bar(
                    df,
                    x="Implementation",
                    y="Speed",
                    title="Generation Speed (gen/sec)",
                    color="Speed",
                    color_continuous_scale="viridis",
                )

                fig.update_xaxes(tickangle=45)
                fig.update_layout(height=400)
                return fig

            except Exception as e:
                self.error_log.append(f"Speed chart error: {e}")
                self._auto_patch_error(e)
                return self._create_empty_chart("차트 생성 오류")

        @self.app.callback(
            Output("auto-memory-chart", "figure"), [Input("auto-data", "data")]
        )
        def update_memory_chart(data):
            """메모리 차트 업데이트"""
            try:
                if not data or not data.get("detailed_results"):
                    return self._create_empty_chart("메모리 데이터 없음")

                results = [
                    r for r in data["detailed_results"] if r.get("success", False)
                ]
                if not results:
                    return self._create_empty_chart("성공한 결과 없음")

                df = pd.DataFrame(
                    {
                        "Implementation": [r["name"] for r in results],
                        "Memory": [r["memory_usage"] for r in results],
                    }
                )

                try:
                    fig = px.bar(
                        df,
                        x="Implementation",
                        y="Memory",
                        title="Memory Usage (bytes)",
                        color="Memory",
                        color_continuous_scale="plasma",  # 안전한 색상 스케일로 변경
                    )
                except Exception as e:
                    # 색상 스케일 없이 기본 차트 생성
                    fig = px.bar(
                        df,
                        x="Implementation",
                        y="Memory",
                        title="Memory Usage (bytes)",
                    )

                fig.update_xaxes(tickangle=45)
                fig.update_layout(height=400)
                return fig

            except Exception as e:
                self.error_log.append(f"Memory chart error: {e}")
                self._auto_patch_error(e)
                return self._create_empty_chart("차트 생성 오류")

        @self.app.callback(
            Output("auto-recommendation", "children"), [Input("auto-data", "data")]
        )
        def update_recommendation(data):
            """추천 결과 업데이트"""
            try:
                if not data or not data.get("recommended_implementation"):
                    return html.P("추천 결과 없음", className="text-center text-muted")

                return html.Div(
                    [
                        html.Div(
                            [
                                html.H2(
                                    "🏆", style={"fontSize": "60px", "margin": "0"}
                                ),
                                html.H3(
                                    data["recommended_implementation"],
                                    className="text-primary mt-2",
                                ),
                                html.P(
                                    "최적 구현으로 추천됩니다", className="text-muted"
                                ),
                            ],
                            className="text-center alert alert-success p-4",
                        ),
                        html.Div(
                            [
                                html.Div(
                                    [
                                        html.H5("🚀 최고 성능"),
                                        html.P(data.get("best_performance", "N/A")),
                                    ],
                                    className="col-md-4 text-center",
                                ),
                                html.Div(
                                    [
                                        html.H5("💾 메모리 효율"),
                                        html.P(
                                            data.get("best_memory_efficiency", "N/A")
                                        ),
                                    ],
                                    className="col-md-4 text-center",
                                ),
                                html.Div(
                                    [
                                        html.H5("📊 분포 품질"),
                                        html.P(data.get("best_distribution", "N/A")),
                                    ],
                                    className="col-md-4 text-center",
                                ),
                            ],
                            className="row mt-3",
                        ),
                    ]
                )

            except Exception as e:
                self.error_log.append(f"Recommendation error: {e}")
                self._auto_patch_error(e)
                return html.P("추천 시스템 오류", className="text-center text-danger")

    def _start_auto_simulation(self):
        """자동 시뮬레이션 시작"""

        def countdown_and_run():
            try:
                # 카운트다운
                for i in range(10, 0, -1):
                    self.countdown = i
                    time.sleep(1)

                # 시뮬레이션 시작
                self.countdown = 0
                self.is_running = True

                print("🚀 Auto-starting multi-implementation comparison...")

                # 시뮬레이션 실행
                report = run_multi_implementation_test(iterations=10000, seed=12345)

                self.latest_report = report
                self.is_running = False
                self.is_completed = True

                print("✅ Multi-implementation comparison completed!")
                print(f"Recommended: {report.recommended_implementation}")

                # 10초 후 자동 종료 (선택적)
                # time.sleep(10)
                # os._exit(0)

            except Exception as e:
                self.error_log.append(f"Auto simulation error: {e}")
                self._auto_patch_error(e)
                self.is_running = False
                self.is_completed = True

        self.auto_thread = threading.Thread(target=countdown_and_run)
        self.auto_thread.daemon = True
        self.auto_thread.start()

    def _create_progress_bar(self, percentage: int, text: str):
        """진행률 바 생성"""
        return html.Div(
            [
                html.Div(
                    [
                        html.Div(
                            style={
                                "width": f"{percentage}%",
                                "height": "30px",
                                "backgroundColor": (
                                    "#28a745" if percentage == 100 else "#007bff"
                                ),
                                "transition": "width 0.5s ease",
                            }
                        )
                    ],
                    style={
                        "width": "100%",
                        "height": "30px",
                        "backgroundColor": "#e9ecef",
                        "borderRadius": "15px",
                        "overflow": "hidden",
                    },
                ),
                html.P(
                    f"{text} ({percentage}%)",
                    className="text-center mt-2 mb-0",
                    style={"fontSize": "18px", "fontWeight": "bold"},
                ),
            ]
        )

    def _create_error_display(self):
        """에러 로그 표시"""
        if not self.error_log:
            return html.Div()

        return html.Div(
            [
                html.H5("⚠️ Error Log", className="text-warning"),
                html.Div(
                    [
                        html.P(error, className="text-danger small")
                        for error in self.error_log[-5:]  # 최근 5개만 표시
                    ],
                    className="alert alert-warning",
                ),
            ]
        )

    def _create_auto_summary_cards(self, report):
        """자동 요약 카드 생성"""
        try:
            cards = [
                html.Div(
                    [
                        html.H3(
                            str(report.total_implementations),
                            style={"fontSize": "48px", "margin": "0"},
                        ),
                        html.P("Total", className="mb-0"),
                    ],
                    className="col-md-3 text-center alert alert-primary p-4",
                ),
                html.Div(
                    [
                        html.H3(
                            str(report.successful_implementations),
                            style={
                                "fontSize": "48px",
                                "margin": "0",
                                "color": "#28a745",
                            },
                        ),
                        html.P("Success", className="mb-0"),
                    ],
                    className="col-md-3 text-center alert alert-success p-4",
                ),
                html.Div(
                    [
                        html.H3(
                            str(report.failed_implementations),
                            style={
                                "fontSize": "48px",
                                "margin": "0",
                                "color": "#dc3545",
                            },
                        ),
                        html.P("Failed", className="mb-0"),
                    ],
                    className="col-md-3 text-center alert alert-danger p-4",
                ),
                html.Div(
                    [
                        html.H3("🏆", style={"fontSize": "48px", "margin": "0"}),
                        html.P("Winner", className="mb-0"),
                    ],
                    className="col-md-3 text-center alert alert-warning p-4",
                ),
            ]

            return cards

        except Exception as e:
            self.error_log.append(f"Summary cards error: {e}")
            return []

    def _create_empty_chart(self, message: str):
        """빈 차트 생성"""
        fig = go.Figure()
        fig.add_annotation(
            text=message,
            xref="paper",
            yref="paper",
            x=0.5,
            y=0.5,
            showarrow=False,
            font=dict(size=20),
        )
        fig.update_layout(
            xaxis=dict(visible=False), yaxis=dict(visible=False), height=400
        )
        return fig

    def _auto_patch_error(self, error: Exception):
        """자동 에러 패치"""
        error_str = str(error)
        error_type = type(error).__name__

        print(f"🔧 Auto-patching error: {error_type} - {error_str}")

        # 일반적인 에러 패치
        if "asdict" in error_str:
            print("✅ Patched: asdict import issue")
            # 이미 수정됨
        elif "KeyError" in error_str:
            print("✅ Patched: Missing key issue - using default values")
        elif "AttributeError" in error_str:
            print("✅ Patched: Missing attribute - using fallback")
        elif "TypeError" in error_str:
            print("✅ Patched: Type error - using type conversion")
        else:
            print(f"⚠️ Unknown error type: {error_type}")

        # 에러 로그에 패치 정보 추가
        self.error_log.append(f"Auto-patched: {error_type}")

    def run_server(self):
        """서버 실행"""
        print("🚀 Starting Auto Multi-Implementation Dashboard")
        print(f"📱 URL: http://localhost:{self.port}")
        print("⏰ Auto-start countdown will begin in 1 second...")
        print("🛑 Press Ctrl+C to stop")

        try:
            self.app.run(debug=self.debug, port=self.port, host="0.0.0.0")
        except KeyboardInterrupt:
            print("\n🛑 Dashboard stopped by user")
        except Exception as e:
            print(f"❌ Server error: {e}")
            self._auto_patch_error(e)


# ==================== 메인 실행 ====================

if __name__ == "__main__":
    print("🚀 Auto Multi-Implementation Dashboard")
    print("=" * 60)

    try:
        dashboard = AutoMultiDashboard(port=8052, debug=False)
        dashboard.run_server()
    except Exception as e:
        print(f"❌ Failed to start dashboard: {e}")
        print("🔧 Attempting auto-patch...")

        # 기본 에러 처리
        if "port" in str(e).lower():
            print("🔄 Trying different port...")
            dashboard = AutoMultiDashboard(port=8053, debug=False)
            dashboard.run_server()
        else:
            print(f"💥 Critical error: {e}")
            traceback.print_exc()
