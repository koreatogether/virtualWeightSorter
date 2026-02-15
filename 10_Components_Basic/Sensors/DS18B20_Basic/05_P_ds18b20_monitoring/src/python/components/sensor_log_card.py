"""센서 로그 카드 컴포넌트

센서별 상세 로그 정보를 표시하는 카드 컴포넌트입니다.
"""

from dataclasses import dataclass
from datetime import datetime
from typing import Any

from dash import html


@dataclass
class SensorDetailInfo:
    """센서 상세 정보를 담는 데이터 클래스"""

    last_temp: float
    avg_temp: float
    temp_range: str
    change_rate: float
    success_rate: str
    consecutive: int
    max_consecutive: int
    failures: int
    uptime: str
    total_measurements: int
    last_failure_time: float | None
    high_violations: int
    low_violations: int


def _format_uptime(uptime_seconds: int) -> str:
    """가동시간을 HH:MM:SS 형식으로 포맷"""
    hours = uptime_seconds // 3600
    minutes = (uptime_seconds % 3600) // 60
    seconds = uptime_seconds % 60
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}"


def _format_success_rate(successful: int, total: int) -> str:
    """성공률을 백분율로 포맷"""
    if total == 0:
        return "0.0%"
    return f"{(successful / total * 100):.1f}%"


def _format_temperature_range(min_temp: float, max_temp: float) -> str:
    """온도 범위 포맷"""
    if min_temp == float("inf") or max_temp == float("-inf"):
        return "N/A"
    return f"{min_temp:.1f}°C ~ {max_temp:.1f}°C"


def _get_status_indicator(
    consecutive_successes: int, connection_failures: int
) -> tuple[str, str]:
    """상태 표시기 반환 (아이콘, 색상)"""
    if consecutive_successes > 5:
        return "🟢", "#28a745"  # 녹색 - 정상
    elif consecutive_successes > 0:
        return "🟡", "#ffc107"  # 노란색 - 주의
    else:
        return "🔴", "#dc3545"  # 빨간색 - 오류


def _create_log_header(
    sensor_id: str, sensor_addr: str, status_icon: str, status_color: str
) -> html.Div:
    """로그 헤더 생성"""
    return html.Div(
        children=[
            html.Div(
                children=[
                    html.Span(status_icon, style={"marginRight": "5px"}),
                    html.Span(f"센서 {sensor_id}", className="log-sensor-id"),
                    html.Span(f"({sensor_addr[-8:]})", className="log-sensor-addr"),
                ],
                className="log-header-title",
            ),
        ],
        className="log-header",
        style={"borderLeft": f"4px solid {status_color}"},
    )


def _create_detailed_info(info: SensorDetailInfo) -> html.Div:
    """상세한 센서 정보 섹션 생성"""

    # 온도 변화 방향 표시
    change_icon = (
        "↗️" if info.change_rate > 0.1 else "↘️" if info.change_rate < -0.1 else "➡️"
    )

    # 마지막 장애 시간 포맷
    failure_time_str = "없음"
    if info.last_failure_time:
        failure_dt = datetime.fromtimestamp(info.last_failure_time)
        failure_time_str = failure_dt.strftime("%H:%M:%S")

    # 임계값 상태
    threshold_status = "정상 범위 내"
    if info.high_violations > 0 or info.low_violations > 0:
        threshold_status = f"⚠️ 위반 {info.high_violations + info.low_violations}회"

    return html.Div(
        children=[
            # 온도 정보
            html.Div(
                [
                    html.Div("🌡️ 온도 정보", className="detail-section-title"),
                    html.Div(
                        [
                            html.Div(
                                f"현재: {info.last_temp:.1f}°C {change_icon}{abs(info.change_rate):.1f}°C/min"
                            ),
                            html.Div(f"평균: {info.avg_temp:.1f}°C"),
                            html.Div(f"범위: {info.temp_range}"),
                        ],
                        className="detail-section-content",
                    ),
                ],
                className="detail-section",
            ),
            # 통신 상태
            html.Div(
                [
                    html.Div("📡 통신 상태", className="detail-section-title"),
                    html.Div(
                        [
                            html.Div(f"성공률: {info.success_rate}"),
                            html.Div(f"연속성공: {info.consecutive}회"),
                            html.Div(f"최대연속: {info.max_consecutive}회"),
                            html.Div(f"장애횟수: {info.failures}회"),
                        ],
                        className="detail-section-content",
                    ),
                ],
                className="detail-section",
            ),
            # 시스템 정보
            html.Div(
                [
                    html.Div("⏱️ 시스템 정보", className="detail-section-title"),
                    html.Div(
                        [
                            html.Div(f"가동시간: {info.uptime}"),
                            html.Div(f"총 측정: {info.total_measurements}회"),
                            html.Div(f"마지막 장애: {failure_time_str}"),
                        ],
                        className="detail-section-content",
                    ),
                ],
                className="detail-section",
            ),
            # 임계값 상태
            html.Div(
                [
                    html.Div("✅ 임계값 상태", className="detail-section-title"),
                    html.Div(threshold_status, className="detail-section-content"),
                ],
                className="detail-section",
            ),
        ],
        className="detailed-info-container",
    )


def _create_sensor_log_entry(sensor_stats: dict[str, Any], timestamp: str) -> html.Div:
    """개별 센서 로그 엔트리 생성"""
    sensor_stats.get("sensor_id", "00")
    last_temp = sensor_stats.get("last_temperature", 0.0)
    last_humidity = sensor_stats.get("last_humidity", 0.0)
    consecutive_successes = sensor_stats.get("consecutive_successes", 0)
    connection_failures = sensor_stats.get("connection_failures", 0)

    status_icon, status_color = _get_status_indicator(
        consecutive_successes, connection_failures
    )

    return html.Div(
        children=[
            html.Span(f"[{timestamp}]", className="log-timestamp"),
            html.Span(status_icon, className="log-status-icon"),
            html.Span(f"{last_temp:.1f}°C", className="log-temp"),
            html.Span(f"{last_humidity:.1f}%", className="log-humidity"),
            html.Span(f"연속성공: {consecutive_successes}회", className="log-success"),
        ],
        className="log-entry",
        style={"borderLeft": f"3px solid {status_color}"},
    )


def sensor_log_card(
    sensors_stats: dict[str, dict[str, Any]] | None = None,
    system_start_time: float | None = None,
    id_prefix: str = "log",
) -> html.Div:
    """센서 로그 카드 생성 - 단일 창에서 모든 센서 정보 표시

    Args:
        sensors_stats: 모든 센서의 통계 데이터 딕셔너리
        system_start_time: 시스템 시작 시간 (timestamp)
        id_prefix: 요소 ID 접두사

    Returns:
        센서 로그 카드 Div 컴포넌트
    """

    # 기본값 설정
    if not sensors_stats:
        return html.Div(
            children=[
                html.Div(
                    children=[
                        html.Div(
                            children=[
                                html.Div(
                                    "센서 데이터를 기다리는 중...",
                                    className="log-waiting",
                                ),
                            ],
                            className="log-scroll-area",
                        ),
                        html.Div("📊 센서 로그", className="log-title-right"),
                    ],
                    className="log-content-container",
                )
            ],
            id=f"{id_prefix}-log-card",
            className="card sensor-log-card",
        )

    # 현재 시간 생성
    current_time = datetime.now().strftime("%H:%M:%S")

    # 센서 ID 순으로 정렬
    sorted_sensors = sorted(
        sensors_stats.items(),
        key=lambda x: (
            x[1].get("sensor_id", "00") == "00",
            x[1].get("sensor_id", "00"),
        ),
    )

    # 로그 엔트리들 생성
    log_entries = []
    for _addr, stats in sorted_sensors:
        if stats.get("sensor_id", "00") != "00":  # 유효한 센서만 표시
            entry = _create_sensor_log_entry(stats, current_time)
            log_entries.append(entry)

    if not log_entries:
        log_entries = [
            html.Div("유효한 센서 데이터가 없습니다.", className="log-no-data")
        ]

    return html.Div(
        children=[
            html.Div(
                children=[
                    html.Div(
                        children=log_entries,
                        className="log-scroll-area",
                        id=f"{id_prefix}-scroll-area",
                    ),
                    html.Div("📊 센서 로그", className="log-title-right"),
                ],
                className="log-content-container",
            )
        ],
        id=f"{id_prefix}-log-card",
        className="card sensor-log-card",
    )


def multi_sensor_log_cards(
    sensors_stats: dict[str, dict[str, Any]],
    system_start_time: float | None = None,
    max_cards: int = 3,
) -> html.Div:
    """다중 센서 로그 카드들 생성

    Args:
        sensors_stats: 센서별 통계 데이터 딕셔너리
        system_start_time: 시스템 시작 시간
        max_cards: 최대 표시할 카드 수

    Returns:
        다중 센서 로그 카드 컨테이너
    """

    if not sensors_stats:
        return html.Div(
            children=[
                html.Div("📊 센서 로그", className="log-title"),
                html.Div("센서 데이터를 기다리는 중...", className="log-waiting"),
            ],
            className="multi-log-container",
        )

    # 센서 ID 순으로 정렬 (00이 아닌 것들 우선)
    sorted(
        sensors_stats.items(),
        key=lambda x: (
            x[1].get("sensor_id", "00") == "00",
            x[1].get("sensor_id", "00"),
        ),
    )

    # 단일 로그 카드 생성 (모든 센서 데이터를 하나의 카드에 표시)
    card = sensor_log_card(
        sensors_stats=sensors_stats,
        system_start_time=system_start_time,
        id_prefix="multi-sensor",
    )

    return card
