"""Import shim for offline_simulator used only in tests.

The test adds the tests/ directory to sys.path then performs:
	from src.python.simulator.offline_simulator import OfflineDS18B20Simulator

This shim recreates a pseudo-package "src.python.simulator" under the tests
namespace that forwards to the real implementation in the project root.
"""

import sys
from pathlib import Path

# Compute project root = tests/.. (parent of tests directory)
shim_file = Path(__file__).resolve()
tests_dir = shim_file.parents[3]  # .../tests
project_root = tests_dir.parent
real_module_path = project_root / "src" / "python" / "simulator"
if str(real_module_path) not in sys.path:
    sys.path.insert(0, str(real_module_path.parent.parent))  # add src parent

from simulator.offline_simulator import (
    OfflineDS18B20Simulator,  # type: ignore
)

__all__ = ["OfflineDS18B20Simulator"]
