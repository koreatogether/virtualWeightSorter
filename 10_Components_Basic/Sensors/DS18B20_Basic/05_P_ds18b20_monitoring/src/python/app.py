"""Dash app module.

Defines the Dash `app` object and the minimal layout used by the
project. This module intentionally contains no runtime/startup code so
it can be safely imported by tests and other scripts.
"""

from dash import Dash

from .callbacks.connection_callbacks import register_connection_callbacks
from .callbacks.register_callbacks import register_callbacks
from .layouts.main_layout import get_layout

app = Dash(__name__)

# Compose layout from the layouts package
app.layout = get_layout()

# Register callbacks (kept idempotent and import-safe)
register_callbacks(app)
register_connection_callbacks(app)
