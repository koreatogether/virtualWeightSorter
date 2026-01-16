"""
Multi-Implementation Arduino Dashboard
여러 Arduino 구현을 동시에 비교하는 대시보드

주요 기능:
- 다중 구현 동시 실행
- 실시간 성능 비교
- 구현별 결과 분석
- 최적 구현 추천
- 인터랙티브 차트
"""

import threading
from dataclasses import asdict
from typing import List

import dash
import pandas as pd
import plotly.express as px
from dash import Input, Output, State, callback_context, dash_table, dcc, html
from multi_implementation_sim import (
    MultiImplementationSimulator,
    run_multi_implementation_test,
)


class MultiImplementationDashboard:
    """다중 구현 비교 대시보드"""

    def __init__(self, port: int = 8051, debug: bool = True):
        self.app = dash.Dash(__name__)
        self.port = port
        self.debug = debug

        # 시뮬레이터 초기화
        self.simulator = MultiImplementationSimulator()

        # 상태 관리
        self.is_running = False
        self.latest_report = None
        self.simulation_thread = None

        # 레이아웃 및 콜백 설정
        self._setup_layout()
        self._setup_callbacks()

        print(f"Multi-Implementation Dashboard initialized on port {port}")

    def _setup_layout(self):
        """대시보드 레이아웃 설정"""
        self.app.layout = html.Div(
            [
                # 헤더
                html.Div(
                    [
                        html.H1(
                            "Arduino Multi-Implementation Comparison Dashboard",
                            className="text-center mb-4",
                        ),
                        html.P(
                            "Compare multiple Arduino random number generator implementations",
                            className="text-center text-muted",
                        ),
                        html.Hr(),
                    ]
                ),
                # 제어 패널
                html.Div(
                    [
                        html.H3("Simulation Control"),
                        html.Div(
                            [
                                html.Div(
                                    [
                                        html.Label("Iterations per Implementation:"),
                                        dcc.Input(
                                            id="iterations-input",
                                            type="number",
                                            value=10000,
                                            min=1000,
                                            max=100000,
                                            step=1000,
                                            className="form-control",
                                        ),
                                    ],
                                    className="col-md-3",
                                ),
                                html.Div(
                                    [
                                        html.Label("Random Seed:"),
                                        dcc.Input(
                                            id="seed-input",
                                            type="number",
                                            value=12345,
                                            className="form-control",
                                        ),
                                    ],
                                    className="col-md-3",
                                ),
                                html.Div(
                                    [
                                        html.Label("Actions:"),
                                        html.Br(),
                                        html.Button(
                                            "Run All Implementations",
                                            id="run-all-btn",
                                            className="btn btn-primary mr-2",
                                        ),
                                        html.Button(
                                            "Stop",
                                            id="stop-btn",
                                            className="btn btn-danger mr-2",
                                            disabled=True,
                                        ),
                                        html.Button(
                                            "Refresh Config",
                                            id="refresh-btn",
                                            className="btn btn-secondary",
                                        ),
                                    ],
                                    className="col-md-6",
                                ),
                            ],
                            className="row",
                        ),
                    ],
                    className="card card-body mb-4",
                ),
                # 구현 목록
                html.Div(
                    [
                        html.H4("Available Implementations"),
                        html.Div(id="implementations-list"),
                    ],
                    className="mb-4",
                ),
                # 진행 상태
                html.Div(
                    [
                        html.H4("Execution Status"),
                        html.Div(id="status-display", className="alert alert-info"),
                        dcc.Interval(
                            id="status-interval", interval=2000, n_intervals=0
                        ),
                    ],
                    className="mb-4",
                ),
                # 요약 카드
                html.Div(
                    [
                        html.H3("Comparison Summary"),
                        html.Div(id="summary-cards", className="row"),
                    ],
                    className="mb-4",
                ),
                # 성능 비교 차트
                html.Div(
                    [
                        html.H3("Performance Comparison"),
                        html.Div(
                            [
                                html.Div(
                                    [
                                        html.H4("Generation Speed"),
                                        dcc.Graph(id="speed-comparison-chart"),
                                    ],
                                    className="col-md-6",
                                ),
                                html.Div(
                                    [
                                        html.H4("Memory Usage"),
                                        dcc.Graph(id="memory-comparison-chart"),
                                    ],
                                    className="col-md-6",
                                ),
                            ],
                            className="row",
                        ),
                    ],
                    className="mb-4",
                ),
                # 분포 비교
                html.Div(
                    [
                        html.H4("Distribution Quality Comparison"),
                        dcc.Graph(id="distribution-comparison-chart"),
                    ],
                    className="mb-4",
                ),
                # 제약 조건 준수
                html.Div(
                    [
                        html.H4("Constraint Compliance"),
                        dcc.Graph(id="constraint-comparison-chart"),
                    ],
                    className="mb-4",
                ),
                # 상세 결과 테이블
                html.Div(
                    [
                        html.H4("Detailed Results"),
                        html.Div(id="detailed-results-table"),
                    ],
                    className="mb-4",
                ),
                # 추천 시스템
                html.Div(
                    [
                        html.H4("Recommendation System"),
                        html.Div(id="recommendation-panel"),
                    ],
                    className="mb-4",
                ),
                # 숨겨진 저장소
                dcc.Store(id="comparison-data"),
                dcc.Store(id="simulation-status"),
            ],
            className="container-fluid",
        )

    def _setup_callbacks(self):
        """콜백 함수 설정"""

        @self.app.callback(
            Output("implementations-list", "children"),
            [Input("refresh-btn", "n_clicks")],
        )
        def update_implementations_list(n_clicks):
            """구현 목록 업데이트"""
            implementations = self.simulator.get_implementation_list()

            cards = []
            for impl in implementations:
                card = html.Div(
                    [
                        html.H6(impl["name"], className="card-title"),
                        html.P(impl["description"], className="card-text"),
                        html.Small(f"Type: {impl['type']}", className="text-muted"),
                    ],
                    className="card card-body col-md-4 mb-2",
                )
                cards.append(card)

            return html.Div(cards, className="row")

        @self.app.callback(
            [
                Output("simulation-status", "data"),
                Output("run-all-btn", "disabled"),
                Output("stop-btn", "disabled"),
            ],
            [Input("run-all-btn", "n_clicks"), Input("stop-btn", "n_clicks")],
            [State("iterations-input", "value"), State("seed-input", "value")],
        )
        def control_simulation(run_clicks, stop_clicks, iterations, seed):
            """시뮬레이션 제어"""
            ctx = callback_context
            if not ctx.triggered:
                return {"running": False}, False, True

            button_id = ctx.triggered[0]["prop_id"].split(".")[0]

            if button_id == "run-all-btn" and run_clicks:
                self._start_multi_simulation(iterations, seed)
                return (
                    {"running": True, "iterations": iterations, "seed": seed},
                    True,
                    False,
                )

            elif button_id == "stop-btn" and stop_clicks:
                self._stop_simulation()
                return {"running": False}, False, True

            return {"running": False}, False, True

        @self.app.callback(
            Output("status-display", "children"),
            [
                Input("status-interval", "n_intervals"),
                Input("simulation-status", "data"),
            ],
        )
        def update_status(n_intervals, status_data):
            """상태 표시 업데이트"""
            if not status_data:
                return "Ready to run multi-implementation comparison"

            if status_data.get("running", False):
                if self.is_running:
                    return html.Div(
                        [
                            html.P(
                                f"Running comparison with {status_data['iterations']:,} iterations per implementation"
                            ),
                            html.P(f"Seed: {status_data['seed']}"),
                            html.P("Please wait... This may take a few moments."),
                        ]
                    )
                else:
                    return "Comparison completed"
            else:
                return "Comparison stopped or completed"

        @self.app.callback(
            [Output("comparison-data", "data"), Output("summary-cards", "children")],
            [Input("status-interval", "n_intervals")],
            [State("simulation-status", "data")],
        )
        def update_results(n_intervals, status_data):
            """결과 업데이트"""
            if self.latest_report:
                summary_cards = self._create_summary_cards(self.latest_report)
                return asdict(self.latest_report), summary_cards

            return {}, []

        @self.app.callback(
            Output("speed-comparison-chart", "figure"),
            [Input("comparison-data", "data")],
        )
        def update_speed_chart(data):
            """속도 비교 차트"""
            if not data or not data.get("detailed_results"):
                return px.bar(title="No data available")

            results = [r for r in data["detailed_results"] if r.get("success", False)]
            if not results:
                return px.bar(title="No successful results")

            df = pd.DataFrame(
                {
                    "Implementation": [r["name"] for r in results],
                    "Speed (gen/sec)": [r["generation_rate"] for r in results],
                }
            )

            fig = px.bar(
                df,
                x="Implementation",
                y="Speed (gen/sec)",
                title="Generation Speed Comparison",
                color="Speed (gen/sec)",
                color_continuous_scale="viridis",
            )

            fig.update_xaxes(tickangle=45)
            return fig

        @self.app.callback(
            Output("memory-comparison-chart", "figure"),
            [Input("comparison-data", "data")],
        )
        def update_memory_chart(data):
            """메모리 사용량 비교 차트"""
            if not data or not data.get("detailed_results"):
                return px.bar(title="No data available")

            results = [r for r in data["detailed_results"] if r.get("success", False)]
            if not results:
                return px.bar(title="No successful results")

            df = pd.DataFrame(
                {
                    "Implementation": [r["name"] for r in results],
                    "Memory (bytes)": [r["memory_usage"] for r in results],
                }
            )

            try:
                fig = px.bar(
                    df,
                    x="Implementation",
                    y="Memory (bytes)",
                    title="Memory Usage Comparison",
                    color="Memory (bytes)",
                    color_continuous_scale="plasma",  # 안전한 색상 스케일로 변경
                )
            except Exception as e:
                # 색상 스케일 없이 기본 차트 생성
                fig = px.bar(
                    df,
                    x="Implementation",
                    y="Memory (bytes)",
                    title="Memory Usage Comparison",
                )

            fig.update_xaxes(tickangle=45)
            return fig

        @self.app.callback(
            Output("distribution-comparison-chart", "figure"),
            [Input("comparison-data", "data")],
        )
        def update_distribution_chart(data):
            """분포 품질 비교 차트"""
            if not data or not data.get("detailed_results"):
                return px.bar(title="No data available")

            results = [r for r in data["detailed_results"] if r.get("success", False)]
            if not results:
                return px.bar(title="No successful results")

            # 분포 균등성 점수 계산
            distribution_scores = []
            for result in results:
                dist = result["distribution"]
                total = sum(dist.values()) if dist else 1
                if total > 0:
                    percentages = [count / total for count in dist.values()]
                    # 33.33%에서 얼마나 벗어났는지 (낮을수록 좋음)
                    score = sum(abs(p - 0.3333) for p in percentages)
                    distribution_scores.append(1 / (1 + score))  # 높을수록 좋게 변환
                else:
                    distribution_scores.append(0)

            df = pd.DataFrame(
                {
                    "Implementation": [r["name"] for r in results],
                    "Distribution Quality": distribution_scores,
                }
            )

            try:
                fig = px.bar(
                    df,
                    x="Implementation",
                    y="Distribution Quality",
                    title="Distribution Quality (Higher is Better)",
                    color="Distribution Quality",
                    color_continuous_scale="viridis",  # 안전한 색상 스케일로 변경
                )
            except Exception as e:
                # 색상 스케일 없이 기본 차트 생성
                fig = px.bar(
                    df,
                    x="Implementation",
                    y="Distribution Quality",
                    title="Distribution Quality (Higher is Better)",
                )

            fig.update_xaxes(tickangle=45)
            return fig

        @self.app.callback(
            Output("constraint-comparison-chart", "figure"),
            [Input("comparison-data", "data")],
        )
        def update_constraint_chart(data):
            """제약 조건 준수 비교 차트"""
            if not data or not data.get("detailed_results"):
                return px.bar(title="No data available")

            results = [r for r in data["detailed_results"] if r.get("success", False)]
            if not results:
                return px.bar(title="No successful results")

            df = pd.DataFrame(
                {
                    "Implementation": [r["name"] for r in results],
                    "Violations": [r["constraint_violations"] for r in results],
                    "Status": [
                        "✅ Pass" if r["constraint_violations"] == 0 else "❌ Fail"
                        for r in results
                    ],
                }
            )

            fig = px.bar(
                df,
                x="Implementation",
                y="Violations",
                title="Constraint Violations (Lower is Better)",
                color="Status",
                color_discrete_map={"✅ Pass": "green", "❌ Fail": "red"},
            )

            fig.update_xaxes(tickangle=45)
            return fig

        @self.app.callback(
            Output("detailed-results-table", "children"),
            [Input("comparison-data", "data")],
        )
        def update_results_table(data):
            """상세 결과 테이블"""
            if not data or not data.get("detailed_results"):
                return html.P("No results available")

            results = data["detailed_results"]

            table_data = []
            for result in results:
                if result.get("success", False):
                    table_data.append(
                        {
                            "Implementation": result["name"],
                            "Speed (gen/sec)": f"{result['generation_rate']:,.0f}",
                            "Memory (bytes)": result["memory_usage"],
                            "Violations": result["constraint_violations"],
                            "Status": "✅ Success",
                        }
                    )
                else:
                    table_data.append(
                        {
                            "Implementation": result["name"],
                            "Speed (gen/sec)": "N/A",
                            "Memory (bytes)": "N/A",
                            "Violations": "N/A",
                            "Status": f"❌ {result.get('error_message', 'Failed')}",
                        }
                    )

            return dash_table.DataTable(
                data=table_data,
                columns=[
                    {"name": "Implementation", "id": "Implementation"},
                    {"name": "Speed (gen/sec)", "id": "Speed (gen/sec)"},
                    {"name": "Memory (bytes)", "id": "Memory (bytes)"},
                    {"name": "Violations", "id": "Violations"},
                    {"name": "Status", "id": "Status"},
                ],
                style_cell={"textAlign": "left"},
                style_data_conditional=[
                    {
                        "if": {"filter_query": "{Status} contains ✅"},
                        "backgroundColor": "#d4edda",
                        "color": "black",
                    },
                    {
                        "if": {"filter_query": "{Status} contains ❌"},
                        "backgroundColor": "#f8d7da",
                        "color": "black",
                    },
                ],
            )

        @self.app.callback(
            Output("recommendation-panel", "children"),
            [Input("comparison-data", "data")],
        )
        def update_recommendation(data):
            """추천 시스템 패널"""
            if not data or not data.get("recommended_implementation"):
                return html.P("No recommendation available")

            return html.Div(
                [
                    html.Div(
                        [
                            html.H5("🏆 Recommended Implementation"),
                            html.H4(
                                data["recommended_implementation"],
                                className="text-primary",
                            ),
                            html.P(
                                "Based on weighted scoring of performance, memory efficiency, and reliability"
                            ),
                        ],
                        className="alert alert-success",
                    ),
                    html.Div(
                        [
                            html.Div(
                                [
                                    html.H6("🚀 Best Performance"),
                                    html.P(data.get("best_performance", "N/A")),
                                ],
                                className="col-md-4",
                            ),
                            html.Div(
                                [
                                    html.H6("💾 Best Memory Efficiency"),
                                    html.P(data.get("best_memory_efficiency", "N/A")),
                                ],
                                className="col-md-4",
                            ),
                            html.Div(
                                [
                                    html.H6("📊 Best Distribution"),
                                    html.P(data.get("best_distribution", "N/A")),
                                ],
                                className="col-md-4",
                            ),
                        ],
                        className="row",
                    ),
                ]
            )

    def _create_summary_cards(self, report) -> List[html.Div]:
        """요약 카드 생성"""
        cards = [
            html.Div(
                [
                    html.H4(str(report.total_implementations)),
                    html.P("Total Implementations"),
                ],
                className="col-md-3 text-center alert alert-primary",
            ),
            html.Div(
                [html.H4(str(report.successful_implementations)), html.P("Successful")],
                className="col-md-3 text-center alert alert-success",
            ),
            html.Div(
                [html.H4(str(report.failed_implementations)), html.P("Failed")],
                className="col-md-3 text-center alert alert-danger",
            ),
            html.Div(
                [html.H4("🏆"), html.P(report.recommended_implementation)],
                className="col-md-3 text-center alert alert-warning",
            ),
        ]

        return cards

    def _start_multi_simulation(self, iterations: int, seed: int):
        """다중 구현 시뮬레이션 시작"""

        def run_simulation():
            try:
                self.is_running = True
                report = run_multi_implementation_test(iterations=iterations, seed=seed)
                self.latest_report = report
                print("Multi-implementation comparison completed")
            except Exception as e:
                print(f"Multi-simulation error: {e}")
            finally:
                self.is_running = False

        self.simulation_thread = threading.Thread(target=run_simulation)
        self.simulation_thread.daemon = True
        self.simulation_thread.start()

    def _stop_simulation(self):
        """시뮬레이션 중단"""
        self.is_running = False

    def run_server(self):
        """대시보드 서버 실행"""
        print(
            f"Starting multi-implementation dashboard on http://localhost:{self.port}"
        )
        self.app.run(debug=self.debug, port=self.port, host="0.0.0.0")


# ==================== 편의 함수들 ====================


def create_multi_dashboard(
    port: int = 8051, debug: bool = True
) -> MultiImplementationDashboard:
    """다중 구현 대시보드 생성"""
    return MultiImplementationDashboard(port=port, debug=debug)


def run_multi_dashboard(port: int = 8051, debug: bool = True):
    """다중 구현 대시보드 실행"""
    dashboard = create_multi_dashboard(port=port, debug=debug)
    dashboard.run_server()


# ==================== 메인 실행 ====================

if __name__ == "__main__":
    print("Arduino Multi-Implementation Comparison Dashboard")
    print("=" * 60)
    print("Starting dashboard server...")
    print("Open your browser and go to: http://localhost:8051")
    print("Press Ctrl+C to stop the server")

    try:
        run_multi_dashboard(port=8051, debug=True)
    except KeyboardInterrupt:
        print("\nDashboard server stopped")
    except Exception as e:
        print(f"Error running dashboard: {e}")
