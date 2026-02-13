"""Domain collector.

Collects DS18B20 project domain metrics:
    - protocol_coverage: implemented commands vs expected spec
    - message_type_coverage: message types defined in protocol
    - persistence_rate: EEPROM file presence & key integrity (simple heuristic)
    - drift_rate: placeholder (requires active multi-cycle comparison)

Future expansions:
    - active EEPROM reload test loop
    - configuration drift detection by simulating set/get sequences
    - sensor fidelity sampling harness
"""

from __future__ import annotations

import ast
import json
from pathlib import Path

from .base_collector import BaseCollector, CollectorResult


class DomainCollector(BaseCollector):
    name = "domain"

    def __init__(self, project_root: Path, strict: bool = False) -> None:
        super().__init__(strict=strict)
        self.project_root = project_root

    def collect(self) -> CollectorResult:  # type: ignore[override]
        # Spec (command) definitions (derived from design docs)
        expected_commands: set[str] = {
            "set_config",
            "get_user_data",
            "set_user_data",
            "get_status",
        }
        expected_message_types: set[str] = {
            "sensor_data",
            "command",
            "response",
            "eeprom_status",
        }

        protocol_py = self.project_root / "src" / "python" / "simulator" / "protocol.py"
        implemented_commands: set[str] = set()
        implemented_message_types: set[str] = set()

        try:
            source = protocol_py.read_text(encoding="utf-8")
            tree = ast.parse(source, filename=str(protocol_py))
            for node in ast.walk(tree):
                # Find string constants referencing command comparisons or message type usage
                if isinstance(node, ast.Constant) and isinstance(node.value, str):
                    val = node.value
                    if val in expected_commands:
                        implemented_commands.add(val)
                    if val in expected_message_types:
                        implemented_message_types.add(val)
        except Exception as e:  # pragma: no cover - defensive
            return CollectorResult(error=f"protocol parse error: {e}")

        # Compute coverage ratios
        command_cov = (
            (len(implemented_commands) / len(expected_commands))
            if expected_commands
            else 0.0
        )
        message_type_cov = (
            (len(implemented_message_types) / len(expected_message_types))
            if expected_message_types
            else 0.0
        )

        # EEPROM persistence heuristic: check file & required keys
        eeprom_file = self.project_root / "data" / "eeprom_simulation.json"
        required_keys = {
            "user_id",
            "sensor_id",
            "th_value",
            "tl_value",
            "measurement_interval",
        }
        persistence_rate = 0.0
        if eeprom_file.exists():
            try:
                with eeprom_file.open(encoding="utf-8") as f:
                    data = json.load(f)
                missing = required_keys - data.keys()
                persistence_rate = (
                    1.0 if not missing else 1.0 - (len(missing) / len(required_keys))
                )
            except Exception:  # pragma: no cover
                persistence_rate = 0.0

        # Drift harness (lightweight): load EEPROM -> modify a temp copy -> compare keys.
        # In real system we'd perform set/get roundtrip; here we simulate stability by ensuring
        # required keys remain stable (drift= fraction missing or altered vs required set).
        drift_rate = 0.0
        if eeprom_file.exists():
            try:
                with eeprom_file.open(encoding="utf-8") as f:
                    original = json.load(f)
                simulated = dict(original)
                # simulate a benign write of same values
                missing_after = required_keys - simulated.keys()
                if required_keys:
                    drift_rate = len(missing_after) / len(required_keys)
            except Exception:  # pragma: no cover
                drift_rate = 1.0  # treat unreadable as full drift

        return CollectorResult(
            persistence_rate=round(persistence_rate, 3),
            protocol_coverage=round(command_cov, 3),
            message_type_coverage=round(message_type_cov, 3),
            implemented_commands=sorted(implemented_commands),
            implemented_message_types=sorted(implemented_message_types),
            expected_commands=sorted(expected_commands),
            expected_message_types=sorted(expected_message_types),
            sensor_fidelity=None,  # pending active sampling
            drift_rate=round(drift_rate, 3),
        )
