import sys
from importlib import util
from pathlib import Path

import pytest
from fastapi.testclient import TestClient

# Ensure src/python is on sys.path so test modules can import the package
REPO_ROOT = Path(__file__).resolve().parents[1]
SRC_PATH = REPO_ROOT / "src" / "python"

if str(SRC_PATH) not in sys.path:
    sys.path.insert(0, str(SRC_PATH))


# Create a shared TestClient fixture if tests need a global client
# Import the app the same way tests do to avoid import-time side-effects
BACKEND_MAIN_PATH = REPO_ROOT / "src" / "python" / "backend" / "main.py"
spec = util.spec_from_file_location("backend.main", str(BACKEND_MAIN_PATH))
module = util.module_from_spec(spec)  # type: ignore
assert spec and spec.loader
spec.loader.exec_module(module)  # type: ignore


@pytest.fixture(scope="session")
def client():
    """Session-scoped TestClient for FastAPI app used across tests."""
    app = module.app
    with TestClient(app) as c:
        yield c
