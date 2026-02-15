"""로그 메시지 생성 및 포맷팅 유틸리티"""

from datetime import datetime
from typing import Any

from dash import html


def create_log_entry(message: str, level: str = "info") -> html.Div:
    """로그 엔트리 생성"""
    timestamp = datetime.now().strftime("%H:%M:%S")

    # 레벨별 색상 설정
    color_map = {
        "info": "#2196F3",  # 파란색
        "success": "#4CAF50",  # 초록색
        "warning": "#FF9800",  # 주황색
        "error": "#F44336",  # 빨간색
        "recommendation": "#9C27B0",  # 보라색
    }

    # 메시지에서 레벨 자동 감지
    if "✅" in message or "성공" in message:
        level = "success"
    elif "❌" in message or "실패" in message or "오류" in message:
        level = "error"
    elif "⚠️" in message or "경고" in message:
        level = "warning"
    elif "💡" in message:
        level = "recommendation"

    color = color_map.get(level, "#666666")

    return html.Div(
        [
            html.Span(f"[{timestamp}] ", style={"color": "#888", "fontSize": "12px"}),
            html.Span(message, style={"color": color, "fontSize": "13px"}),
        ],
        style={"marginBottom": "4px", "fontFamily": "monospace", "padding": "2px 4px"},
    )


def create_log_separator() -> html.Hr:
    """로그 구분선 생성"""
    return html.Hr(
        style={"margin": "8px 0", "border": "none", "borderTop": "1px solid #eee"}
    )


def format_connection_status(port: str, is_connected: bool) -> str:
    """연결 상태 메시지 포맷"""
    if is_connected:
        return f"✅ {port} 연결 성공 - Arduino 응답 확인됨"
    else:
        return f"❌ {port} 연결 실패"


def format_diagnostic_message(diagnostic_result: Any) -> list[html.Div]:
    """진단 결과를 로그 메시지 리스트로 변환"""
    messages = []

    if diagnostic_result.success:
        messages.append(create_log_entry(f"✅ {diagnostic_result.message}"))
    else:
        messages.append(create_log_entry(f"❌ {diagnostic_result.message}"))
        if diagnostic_result.recommendation:
            messages.append(create_log_entry(f"💡 {diagnostic_result.recommendation}"))

    return messages
