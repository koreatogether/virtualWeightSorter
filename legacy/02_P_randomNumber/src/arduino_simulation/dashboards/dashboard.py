"""
Arduino Simulation Dashboard
Dash를 사용한 실시간 시뮬레이션 모니터링 및 결과 시각화

주요 기능:
- 실시간 시뮬레이션 실행 및 모니터링
- 분포 분석 시각화
- 제약 조건 검증 결과 표시
- 성능 메트릭 대시보드
- Arduino 하드웨어 정보 표시
"""

import threading
from typing import Any, Dict, List

import dash
import pandas as pd
import plotly.express as px
from dash import Input, Output, State, callback_context, dcc, html

from temp.arduino_mock import ArduinoUnoR4WiFiMock
from tests.simulation_runner import SimulationConfig, SimulationRunner


class ArduinoSimulationDashboard:
    """Arduino 시뮬레이션 대시보드"""

    def __init__(self, port: int = 8050, debug: bool = True):
        self.app = dash.Dash(__name__)
        self.port = port
        self.debug = debug

        # 시뮬레이션 상태
        self.current_simulation = None
        self.simulation_runner = None
        self.simulation_thread = None
        self.is_running = False

        # 결과 저장
        self.latest_results = None
        self.simulation_history = []

        # 레이아웃 설정
        self._setup_layout()
        self._setup_callbacks()

        print(f"Dashboard initialized on port {port}")

    def _setup_layout(self):
        """대시보드 레이아웃 설정"""
        self.app.layout = html.Div(
            [
                # 헤더
                html.Div(
                    [
                        html.H1(
                            "Arduino Uno R4 WiFi Random Number Generator Simulation",
                            className="text-center mb-4",
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
                                        html.Label("Iterations:"),
                                        dcc.Input(
                                            id="iterations-input",
                                            type="number",
                                            value=10000,
                                            min=100,
                                            max=100000,
                                            step=100,
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
                                        html.Label("Action:"),
                                        html.Br(),
                                        html.Button(
                                            "Start Simulation",
                                            id="start-btn",
                                            className="btn btn-primary mr-2",
                                        ),
                                        html.Button(
                                            "Stop",
                                            id="stop-btn",
                                            className="btn btn-danger mr-2",
                                            disabled=True,
                                        ),
                                        html.Button(
                                            "Load Results",
                                            id="load-btn",
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
                # 진행 상태
                html.Div(
                    [
                        html.H4("Simulation Status"),
                        html.Div(id="status-display", className="alert alert-info"),
                        dcc.Interval(
                            id="status-interval", interval=1000, n_intervals=0
                        ),
                    ],
                    className="mb-4",
                ),
                # KPI 카드들
                html.Div(
                    [
                        html.H3("Key Performance Indicators"),
                        html.Div(id="kpi-cards", className="row"),
                    ],
                    className="mb-4",
                ),
                # 차트 섹션
                html.Div(
                    [
                        html.H3("Analysis Charts"),
                        # 분포 차트
                        html.Div(
                            [
                                html.H4("Number Distribution"),
                                dcc.Graph(id="distribution-chart"),
                            ],
                            className="col-md-6",
                        ),
                        # 시퀀스 차트
                        html.Div(
                            [
                                html.H4("Generation Sequence (First 100)"),
                                dcc.Graph(id="sequence-chart"),
                            ],
                            className="col-md-6",
                        ),
                    ],
                    className="row mb-4",
                ),
                # 전이 분석
                html.Div(
                    [html.H4("Transition Analysis"), dcc.Graph(id="transition-chart")],
                    className="mb-4",
                ),
                # 성능 메트릭
                html.Div(
                    [html.H4("Performance Metrics"), html.Div(id="performance-table")],
                    className="mb-4",
                ),
                # Arduino 하드웨어 정보
                html.Div(
                    [
                        html.H4("Arduino Hardware Information"),
                        html.Div(id="hardware-info"),
                    ],
                    className="mb-4",
                ),
                # 숨겨진 저장소
                dcc.Store(id="simulation-data"),
                dcc.Store(id="simulation-status"),
            ],
            className="container-fluid",
        )

    def _setup_callbacks(self):
        """콜백 함수 설정"""

        @self.app.callback(
            [
                Output("simulation-status", "data"),
                Output("start-btn", "disabled"),
                Output("stop-btn", "disabled"),
            ],
            [Input("start-btn", "n_clicks"), Input("stop-btn", "n_clicks")],
            [State("iterations-input", "value"), State("seed-input", "value")],
        )
        def control_simulation(start_clicks, stop_clicks, iterations, seed):
            """시뮬레이션 제어"""
            ctx = callback_context
            if not ctx.triggered:
                return {"running": False}, False, True

            button_id = ctx.triggered[0]["prop_id"].split(".")[0]

            if button_id == "start-btn" and start_clicks:
                # 시뮬레이션 시작
                self._start_simulation(iterations, seed)
                return (
                    {"running": True, "iterations": iterations, "seed": seed},
                    True,
                    False,
                )

            elif button_id == "stop-btn" and stop_clicks:
                # 시뮬레이션 중단
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
                return "Ready to start simulation"

            if status_data.get("running", False):
                if self.simulation_runner and self.simulation_runner.is_running:
                    progress = self.simulation_runner.get_progress()
                    return html.Div(
                        [
                            html.P(
                                f"Running simulation with {status_data['iterations']:,} iterations (seed: {status_data['seed']})"
                            ),
                            html.P(
                                f"Progress: {progress.percentage:.1f}% ({progress.current_iteration:,}/{progress.total_iterations:,})"
                            ),
                            html.P(
                                f"Rate: {progress.generation_rate:,.0f} gen/sec, Remaining: {progress.estimated_remaining:.1f}s"
                            ),
                        ]
                    )
                else:
                    return "Simulation completed"
            else:
                return "Simulation stopped or completed"

        @self.app.callback(
            [Output("simulation-data", "data"), Output("kpi-cards", "children")],
            [Input("status-interval", "n_intervals")],
            [State("simulation-status", "data")],
        )
        def update_results(n_intervals, status_data):
            """결과 업데이트"""
            if self.latest_results:
                kpi_cards = self._create_kpi_cards(self.latest_results)
                return self.latest_results, kpi_cards

            return {}, []

        @self.app.callback(
            Output("distribution-chart", "figure"), [Input("simulation-data", "data")]
        )
        def update_distribution_chart(data):
            """분포 차트 업데이트"""
            if not data or "distribution_analysis" not in data:
                return px.bar(title="No data available")

            dist_data = data["distribution_analysis"]

            df = pd.DataFrame(
                {
                    "Number": list(dist_data["counts"].keys()),
                    "Count": list(dist_data["counts"].values()),
                    "Percentage": list(dist_data["percentages"].values()),
                }
            )

            fig = px.bar(
                df,
                x="Number",
                y="Count",
                title="Number Distribution",
                text="Percentage",
                color="Number",
                color_discrete_sequence=["#FF6B6B", "#4ECDC4", "#45B7D1"],
            )

            fig.update_traces(texttemplate="%{text:.1f}%", textposition="outside")
            fig.update_layout(showlegend=False)
            first_key = next(iter(dist_data["counts"].keys()), None)
            if first_key is not None:
                fig.add_hline(
                    y=dist_data["counts"][first_key] * (100 / 33.33) / 100,
                    line_dash="dash",
                    line_color="red",
                    annotation_text="Expected (33.33%)",
                )

            return fig

        @self.app.callback(
            Output("sequence-chart", "figure"), [Input("simulation-data", "data")]
        )
        def update_sequence_chart(data):
            """시퀀스 차트 업데이트"""
            if not data or "sample_sequence" not in data:
                return px.line(title="No data available")

            sequence = data["sample_sequence"][:100]  # 처음 100개만

            df = pd.DataFrame({"Index": range(len(sequence)), "Number": sequence})

            fig = px.line(
                df,
                x="Index",
                y="Number",
                title="Generation Sequence (First 100)",
                markers=True,
            )

            fig.update_traces(line=dict(width=2), marker=dict(size=4))
            fig.update_yaxes(tickmode="array", tickvals=[0, 1, 2])

            return fig

        @self.app.callback(
            Output("transition-chart", "figure"), [Input("simulation-data", "data")]
        )
        def update_transition_chart(data):
            """전이 차트 업데이트"""
            if not data or "transition_analysis" not in data:
                return px.bar(title="No data available")

            transitions = data["transition_analysis"]["transitions"]

            df = pd.DataFrame(
                {
                    "Transition": list(transitions.keys()),
                    "Count": list(transitions.values()),
                }
            )

            fig = px.bar(
                df,
                x="Transition",
                y="Count",
                title="State Transitions",
                color="Count",
                color_continuous_scale="viridis",
            )

            return fig

        @self.app.callback(
            Output("performance-table", "children"), [Input("simulation-data", "data")]
        )
        def update_performance_table(data):
            """성능 테이블 업데이트"""
            if not data or "performance_metrics" not in data:
                return html.P("No performance data available")

            perf = data["performance_metrics"]
            sim_info = data.get("simulation_info", {})

            table_data = [
                ["Total Iterations", f"{sim_info.get('total_iterations', 0):,}"],
                ["Total Time", f"{sim_info.get('total_time_seconds', 0):.2f} seconds"],
                [
                    "Generation Rate",
                    f"{sim_info.get('generation_rate_per_second', 0):,.0f} gen/sec",
                ],
                [
                    "Avg Generation Time",
                    f"{perf.get('avg_generation_time_microseconds', 0):.2f} μs",
                ],
                [
                    "Arduino Instructions",
                    f"{perf.get('arduino_instruction_count', 0):,}",
                ],
                ["SRAM Usage", f"{perf.get('sram_usage_percent', 0):.2f}%"],
            ]

            return html.Table(
                [
                    html.Thead([html.Tr([html.Th("Metric"), html.Th("Value")])]),
                    html.Tbody(
                        [
                            html.Tr([html.Td(row[0]), html.Td(row[1])])
                            for row in table_data
                        ]
                    ),
                ],
                className="table table-striped",
            )

        @self.app.callback(
            Output("hardware-info", "children"), [Input("simulation-data", "data")]
        )
        def update_hardware_info(data):
            """하드웨어 정보 업데이트"""
            # Arduino Mock에서 하드웨어 정보 가져오기
            arduino = ArduinoUnoR4WiFiMock()
            hw_info = arduino.get_hardware_info()

            info_cards = []
            for key, value in hw_info.items():
                info_cards.append(
                    html.Div(
                        [html.H6(key.replace("_", " ").title()), html.P(str(value))],
                        className="col-md-3 text-center",
                    )
                )

            return html.Div(info_cards, className="row")

    def _create_kpi_cards(self, data: Dict[str, Any]) -> List[html.Div]:
        """KPI 카드 생성"""
        if not data:
            return []

        sim_info = data.get("simulation_info", {})
        constraint = data.get("constraint_verification", {})
        dist_analysis = data.get("distribution_analysis", {})
        perf = data.get("performance_metrics", {})

        cards = [
            # 총 생성 횟수
            html.Div(
                [
                    html.H4(f"{sim_info.get('total_iterations', 0):,}"),
                    html.P("Total Generations"),
                ],
                className="col-md-3 text-center alert alert-primary",
            ),
            # 생성 속도
            html.Div(
                [
                    html.H4(f"{sim_info.get('generation_rate_per_second', 0):,.0f}"),
                    html.P("Gen/Second"),
                ],
                className="col-md-3 text-center alert alert-info",
            ),
            # 제약 조건 만족
            html.Div(
                [
                    html.H4(
                        "✅" if constraint.get("constraint_satisfied", False) else "❌"
                    ),
                    html.P("Constraint Satisfied"),
                ],
                className=(
                    "col-md-3 text-center alert alert-success"
                    if constraint.get("constraint_satisfied", False)
                    else "col-md-3 text-center alert alert-danger"
                ),
            ),
            # SRAM 사용률
            html.Div(
                [
                    html.H4(f"{perf.get('sram_usage_percent', 0):.1f}%"),
                    html.P("SRAM Usage"),
                ],
                className="col-md-3 text-center alert alert-warning",
            ),
        ]

        return cards

    def _start_simulation(self, iterations: int, seed: int):
        """시뮬레이션 시작"""
        config = SimulationConfig(
            iterations=iterations, seed=seed, show_progress=True, save_results=True
        )

        self.simulation_runner = SimulationRunner(config)

        def run_simulation():
            try:
                results = self.simulation_runner.run_single_simulation()
                self.latest_results = results
                self.simulation_history.append(results)
                print("Simulation completed successfully")
            except Exception as e:
                print(f"Simulation error: {e}")
            finally:
                self.is_running = False

        self.simulation_thread = threading.Thread(target=run_simulation)
        self.simulation_thread.daemon = True
        self.simulation_thread.start()
        self.is_running = True

    def _stop_simulation(self):
        """시뮬레이션 중단"""
        if self.simulation_runner:
            self.simulation_runner.stop_simulation()
        self.is_running = False

    def run_server(self):
        """대시보드 서버 실행"""
        print(f"Starting dashboard server on http://localhost:{self.port}")
        self.app.run(debug=self.debug, port=self.port, host="0.0.0.0")


# ==================== 편의 함수들 ====================


def create_dashboard(
    port: int = 8050, debug: bool = True
) -> ArduinoSimulationDashboard:
    """대시보드 생성 편의 함수"""
    return ArduinoSimulationDashboard(port=port, debug=debug)


def run_dashboard(port: int = 8050, debug: bool = True):
    """대시보드 실행 편의 함수"""
    dashboard = create_dashboard(port=port, debug=debug)
    dashboard.run_server()


# ==================== 메인 실행 ====================

if __name__ == "__main__":
    print("Arduino Uno R4 WiFi Simulation Dashboard")
    print("=" * 50)
    print("Starting dashboard server...")
    print("Open your browser and go to: http://localhost:8050")
    print("Press Ctrl+C to stop the server")

    try:
        run_dashboard(port=8050, debug=True)
    except KeyboardInterrupt:
        print("\nDashboard server stopped")
    except Exception as e:
        print(f"Error running dashboard: {e}")
