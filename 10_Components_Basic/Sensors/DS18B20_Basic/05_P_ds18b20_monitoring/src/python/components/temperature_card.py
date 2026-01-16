from dash import html


def _validate_sensor_id(sensor_id: str | None) -> tuple[str, str]:
    """센서 ID 검증 및 CSS 클래스 반환"""
    valid_ids = {f"{i:02d}" for i in range(0, 9)}
    sid = sensor_id if sensor_id in valid_ids else "00"
    id_error = "" if sensor_id in valid_ids or sensor_id is None else " id-error"
    return sid, id_error


def _format_temperature(temperature: float | None) -> str:
    """온도 값 포맷팅"""
    return "125.0°C" if temperature is None else f"{temperature:.1f}°C"


def _format_address_group(group: str) -> str:
    """8자리 주소 그룹을 XXXX:XXXX 형태로 포맷"""
    left = group[:4]
    right = group[4:8]
    return f"{left}:{right}" if right else left


def _format_address(address: str | None) -> list[str]:
    """주소를 2그룹으로 분할 및 포맷"""
    placeholder = "0X0X0X0X0D0D0D0D"
    addr = (address or placeholder).replace(" ", "")
    # Ensure at least 16 characters (pad with zeros) then take first 16
    addr = (addr + "0" * 16)[:16]
    groups = [addr[i : i + 8] for i in range(0, 16, 8)]
    return [_format_address_group(group) for group in groups]


def _create_sensor_id_row(sid: str, id_error: str, id_prefix: str) -> html.Div:
    """센서 ID 행 생성"""
    return html.Div(
        [
            html.Span("센서 ID:", className="sensor-id-label"),
            html.Span(
                sid,
                id=f"{id_prefix}-sensor-id",
                className=f"sensor-id-value{id_error}",
            ),
        ],
        className="sensor-id-row",
    )


def _create_temperature_row(temp_display: str, id_prefix: str) -> html.Div:
    """온도 값 행 생성"""
    return html.Div(temp_display, id=f"{id_prefix}-temp-value", className="temp-value")


def _create_address_block(address_lines: list[str], id_prefix: str) -> html.Div:
    """주소 블록 생성"""
    return html.Div(
        children=[
            html.Div(address_lines[0], id=f"{id_prefix}-addr-0", className="addr-line"),
            html.Div(address_lines[1], id=f"{id_prefix}-addr-1", className="addr-line"),
        ],
        className="addr-block",
    )


def temperature_card(
    sensor_id: str | None = None,
    temperature: float | None = None,
    address: str | None = None,
    id_prefix: str = "main",
) -> html.Div:
    """Return a temperature card showing sensor id, temperature and address.

    The card structure provides element ids so callbacks can update values in
    real time. Arguments are optional and used as initial/placeholder values.

    - sensor_id: expected '00','01'..'08'. If None or invalid, shows '00' and
      a CSS class `id-error` is added for logging/visuals.
    - temperature: float value; displayed with 1 decimal place or '--' if None.
    - address: full sensor address string; displayed split into 4 groups of 4
      characters across 4 lines.
    - id_prefix: a prefix to make element ids reasonably unique when multiple
      cards are used on the page.
    """
    sid, id_error = _validate_sensor_id(sensor_id)
    temp_display = _format_temperature(temperature)
    address_lines = _format_address(address)

    return html.Div(
        children=[
            _create_sensor_id_row(sid, id_error, id_prefix),
            _create_temperature_row(temp_display, id_prefix),
            _create_address_block(address_lines, id_prefix),
        ],
        className="card temperature-card",
    )
