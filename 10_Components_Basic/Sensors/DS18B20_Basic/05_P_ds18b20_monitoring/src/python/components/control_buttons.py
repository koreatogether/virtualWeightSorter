from dash import html


def control_buttons() -> html.Div:
    """Return a vertical column with three control buttons (100px wide)."""
    return html.Div(
        children=[
            html.Button("ID 변경", id="btn-id", className="control-button"),
            html.Button("TH/TL", id="btn-thtl", className="control-button"),
            html.Button("측정주기", id="btn-interval", className="control-button"),
        ],
        className="control-column",
    )
