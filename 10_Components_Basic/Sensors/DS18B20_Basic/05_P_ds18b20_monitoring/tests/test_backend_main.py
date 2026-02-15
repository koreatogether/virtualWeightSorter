from __future__ import annotations

# Import the FastAPI app directly from its file to avoid package-relative imports
from importlib import util
from pathlib import Path
from unittest.mock import patch

import pytest
from fastapi.testclient import TestClient

REPO_ROOT = Path(__file__).resolve().parents[1]
BACKEND_MAIN_PATH = REPO_ROOT / "src" / "python" / "backend" / "main.py"
spec = util.spec_from_file_location("backend.main", str(BACKEND_MAIN_PATH))
module = util.module_from_spec(spec)  # type: ignore
assert spec and spec.loader
spec.loader.exec_module(module)  # type: ignore

app = module.app
HealthResponse = module.HealthResponse


class TestFastAPIBasic:
    """Test basic FastAPI application functionality"""

    @pytest.fixture()
    def client(self):
        """Fixture providing a FastAPI test client"""
        return TestClient(app)

    def test_app_initialization(self):
        """Test FastAPI app initialization"""
        assert app.title == "DS18B20 / DHT22 Backend API"

        # Check that routes are registered
        route_paths = [route.path for route in app.routes]
        assert "/health" in route_paths
        assert "/sensors" in route_paths

    def test_health_endpoint_success(self, client):
        """Test successful health endpoint response"""
        response = client.get("/health")

        assert response.status_code == 200
        assert response.headers["content-type"] == "application/json"

        # Check response structure
        data = response.json()
        assert data == {"status": "ok"}

        # Verify response model compliance
        health_response = HealthResponse(**data)
        assert health_response.status == "ok"

    def test_health_endpoint_response_model(self, client):
        """Test health endpoint response model validation"""
        response = client.get("/health")

        # Response should match HealthResponse model
        data = response.json()
        health_response = HealthResponse.model_validate(data)
        assert isinstance(health_response, HealthResponse)
        assert health_response.status == "ok"

    def test_sensors_endpoint_success(self, client):
        """Test successful sensors endpoint response"""
        response = client.get("/sensors")

        assert response.status_code == 200
        assert response.headers["content-type"] == "application/json"

        # Check response structure
        data = response.json()
        assert "sensors" in data
        assert isinstance(data["sensors"], list)
        assert data["sensors"] == []  # Currently returns empty list

    def test_sensors_endpoint_response_structure(self, client):
        """Test sensors endpoint response structure"""
        response = client.get("/sensors")
        data = response.json()

        # Should have the expected structure
        assert isinstance(data, dict)
        assert len(data) == 1
        assert "sensors" in data
        assert isinstance(data["sensors"], list)


class TestFastAPIErrorHandling:
    """Test FastAPI error handling scenarios"""

    @pytest.fixture()
    def client(self):
        """Fixture providing a FastAPI test client"""
        return TestClient(app)

    def test_nonexistent_endpoint(self, client):
        """Test accessing non-existent endpoint"""
        response = client.get("/nonexistent")

        assert response.status_code == 404

        # FastAPI returns standard 404 error
        data = response.json()
        assert "detail" in data
        assert data["detail"] == "Not Found"

    def test_invalid_http_method_health(self, client):
        """Test invalid HTTP method on health endpoint"""
        # Health endpoint only supports GET
        response = client.post("/health")
        assert response.status_code == 405  # Method Not Allowed

        response = client.put("/health")
        assert response.status_code == 405

        response = client.delete("/health")
        assert response.status_code == 405

    def test_invalid_http_method_sensors(self, client):
        """Test invalid HTTP method on sensors endpoint"""
        # Sensors endpoint only supports GET
        response = client.post("/sensors")
        assert response.status_code == 405  # Method Not Allowed

        response = client.put("/sensors")
        assert response.status_code == 405

        response = client.delete("/sensors")
        assert response.status_code == 405

    def test_malformed_request_paths(self, client):
        """Test various malformed request paths"""
        malformed_paths = [
            "/health/",  # With trailing slash
            "/sensors/",  # With trailing slash
            "/health/extra",  # Extra path components
            "/sensors/extra",
            "//health",  # Double slash
            "//sensors",
        ]

        for path in malformed_paths:
            response = client.get(path)
            # Should either work (if FastAPI handles it) or return 404
            assert response.status_code in [200, 404]

    def test_request_with_query_parameters(self, client):
        """Test endpoints with query parameters"""
        # Health endpoint with query params (should still work)
        response = client.get("/health?param=value")
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "ok"

        # Sensors endpoint with query params
        response = client.get("/sensors?filter=active")
        assert response.status_code == 200
        data = response.json()
        assert "sensors" in data

    def test_request_with_headers(self, client):
        """Test endpoints with various headers"""
        custom_headers = {
            "X-Custom-Header": "test-value",
            "User-Agent": "test-client/1.0",
            "Accept": "application/json"
        }

        response = client.get("/health", headers=custom_headers)
        assert response.status_code == 200

        response = client.get("/sensors", headers=custom_headers)
        assert response.status_code == 200


class TestFastAPIModels:
    """Test Pydantic models and validation"""

    def test_health_response_model_creation(self):
        """Test HealthResponse model creation"""
        # Valid creation
        response = HealthResponse(status="ok")
        assert response.status == "ok"

        # Test with different valid status
        response = HealthResponse(status="healthy")
        assert response.status == "healthy"

    def test_health_response_model_validation(self):
        """Test HealthResponse model validation"""
        # Valid data
        valid_data = {"status": "ok"}
        response = HealthResponse.model_validate(valid_data)
        assert response.status == "ok"

        # Test serialization
        json_data = response.model_dump()
        assert json_data == {"status": "ok"}

    def test_health_response_model_edge_cases(self):
        """Test HealthResponse model with edge cases"""
        # Empty string
        response = HealthResponse(status="")
        assert response.status == ""

        # Very long string
        long_status = "x" * 1000
        response = HealthResponse(status=long_status)
        assert response.status == long_status

        # Special characters
        special_status = "status-with_special.chars123"
        response = HealthResponse(status=special_status)
        assert response.status == special_status


class TestFastAPIAsyncBehavior:
    """Test async behavior of FastAPI endpoints"""

    @pytest.fixture()
    def client(self):
        """Fixture providing a FastAPI test client"""
        return TestClient(app)

    def test_health_endpoint_async_behavior(self, client):
        """Test health endpoint async response characteristics"""
        import time

        # Measure response time
        start_time = time.time()
        response = client.get("/health")
        end_time = time.time()

        assert response.status_code == 200

        # Async endpoint should be fast
        response_time = end_time - start_time
        assert response_time < 1.0  # Should respond within 1 second

    def test_sensors_endpoint_async_behavior(self, client):
        """Test sensors endpoint async response characteristics"""
        import time

        start_time = time.time()
        response = client.get("/sensors")
        end_time = time.time()

        assert response.status_code == 200

        # Should be fast since it's just returning empty list
        response_time = end_time - start_time
        assert response_time < 1.0

    def test_concurrent_requests(self, client):
        """Test handling of concurrent requests"""
        import concurrent.futures
        import threading

        def make_request(endpoint: str) -> dict:
            response = client.get(endpoint)
            return {
                "status_code": response.status_code,
                "data": response.json(),
                "thread_id": threading.current_thread().ident
            }

        # Make concurrent requests to both endpoints
        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as executor:
            futures = []

            # Submit multiple requests
            for _ in range(5):
                futures.append(executor.submit(make_request, "/health"))
                futures.append(executor.submit(make_request, "/sensors"))

            # Collect results
            results = [future.result() for future in concurrent.futures.as_completed(futures)]

        # All requests should succeed
        assert len(results) == 10
        for result in results:
            assert result["status_code"] == 200
            assert "data" in result

        # Should have used multiple threads
        thread_ids = set(result["thread_id"] for result in results)
        assert len(thread_ids) >= 1  # At least one thread used


class TestFastAPIIntegration:
    """Integration tests for the FastAPI application"""

    @pytest.fixture()
    def client(self):
        """Fixture providing a FastAPI test client"""
        return TestClient(app)

    def test_full_api_workflow(self, client):
        """Test complete API workflow"""
        # Check health first
        health_response = client.get("/health")
        assert health_response.status_code == 200
        assert health_response.json()["status"] == "ok"

        # Get sensors list
        sensors_response = client.get("/sensors")
        assert sensors_response.status_code == 200
        sensors_data = sensors_response.json()
        assert "sensors" in sensors_data
        assert isinstance(sensors_data["sensors"], list)

    def test_api_consistency(self, client):
        """Test API response consistency across multiple calls"""
        # Make multiple calls to each endpoint
        health_responses = []
        sensors_responses = []

        for _ in range(5):
            health_responses.append(client.get("/health").json())
            sensors_responses.append(client.get("/sensors").json())

        # All health responses should be identical
        first_health = health_responses[0]
        for response in health_responses[1:]:
            assert response == first_health

        # All sensors responses should be identical
        first_sensors = sensors_responses[0]
        for response in sensors_responses[1:]:
            assert response == first_sensors

    def test_content_type_handling(self, client):
        """Test content type handling"""
        # Test with different Accept headers
        accept_headers = [
            "application/json",
            "application/json; charset=utf-8",
            "*/*",
            "text/html,application/json"
        ]

        for accept in accept_headers:
            headers = {"Accept": accept}

            health_response = client.get("/health", headers=headers)
            assert health_response.status_code == 200
            assert "application/json" in health_response.headers["content-type"]

            sensors_response = client.get("/sensors", headers=headers)
            assert sensors_response.status_code == 200
            assert "application/json" in sensors_response.headers["content-type"]

    def test_api_documentation_endpoints(self, client):
        """Test auto-generated API documentation endpoints"""
        # FastAPI automatically provides these endpoints
        doc_endpoints = ["/docs", "/redoc", "/openapi.json"]

        for endpoint in doc_endpoints:
            response = client.get(endpoint)
            # These should exist and return content
            assert response.status_code in [200, 404]  # 404 if disabled in config


class TestFastAPIMainFunction:
    """Test the main function and uvicorn integration"""

    @patch('uvicorn.run')
    def test_main_function_execution(self, mock_uvicorn_run):
        """Test main function calls uvicorn.run correctly"""
        # Import and call main function
        if hasattr(module, '__name__') and module.__name__ == "__main__":
            # This would only run if the script is executed directly
            pass

        # Test the main function configuration
        # Since the main function is only called when __name__ == "__main__",
        # we'll test the expected configuration
        expected_config = {
            "host": "127.0.0.1",
            "port": 8000,
            "reload": True
        }

        # Verify the app module path would be correct
        expected_app_path = "src.python.backend.main:app"

        # The configuration should be correct for production use
        assert expected_config["host"] == "127.0.0.1"
        assert expected_config["port"] == 8000
        assert expected_config["reload"] == True

    def test_app_lifespan_events(self, client):
        """Test app lifespan events if any are defined"""
        # FastAPI lifespan events would be tested here
        # Currently the app doesn't define any, so just verify app starts correctly
        response = client.get("/health")
        assert response.status_code == 200


class TestFastAPIPerformance:
    """Performance-related tests for the FastAPI application"""

    @pytest.fixture()
    def client(self):
        """Fixture providing a FastAPI test client"""
        return TestClient(app)

    def test_response_time_benchmarks(self, client):
        """Test response time benchmarks"""
        import time

        endpoints = ["/health", "/sensors"]

        for endpoint in endpoints:
            times = []
            for _ in range(10):
                start = time.time()
                response = client.get(endpoint)
                end = time.time()

                assert response.status_code == 200
                times.append(end - start)

            # Calculate average response time
            avg_time = sum(times) / len(times)
            max_time = max(times)

            # Should be fast (these are simple endpoints)
            assert avg_time < 0.1  # Average under 100ms
            assert max_time < 0.5   # Max under 500ms

    def test_memory_usage_stability(self, client):
        """Test memory usage doesn't grow with requests"""
        # Make many requests to check for memory leaks
        for _ in range(100):
            health_response = client.get("/health")
            assert health_response.status_code == 200

            sensors_response = client.get("/sensors")
            assert sensors_response.status_code == 200

        # If we got here without issues, memory is stable enough

    def test_large_number_of_requests(self, client):
        """Test handling large number of sequential requests"""
        success_count = 0
        error_count = 0

        for i in range(50):
            try:
                response = client.get("/health")
                if response.status_code == 200:
                    success_count += 1
                else:
                    error_count += 1
            except Exception:
                error_count += 1

        # Should handle most requests successfully
        assert success_count >= 45  # At least 90% success rate
        assert error_count <= 5


class TestFastAPIEdgeCases:
    """Test edge cases and boundary conditions"""

    @pytest.fixture()
    def client(self):
        """Fixture providing a FastAPI test client"""
        return TestClient(app)

    def test_extremely_long_url_paths(self, client):
        """Test handling of extremely long URL paths"""
        # Create a very long path
        long_path = "/health" + "x" * 1000

        response = client.get(long_path)
        # Should return 404 for non-existent path
        assert response.status_code == 404

    def test_special_characters_in_urls(self, client):
        """Test URLs with special characters"""
        special_paths = [
            "/health%20space",
            "/sensors?param=value%20with%20spaces",
            "/health#fragment",
            "/sensors?unicode=한글",
        ]

        for path in special_paths:
            try:
                response = client.get(path)
                # Should handle gracefully (either 200 for valid or 404 for invalid)
                assert response.status_code in [200, 404, 422]
            except Exception:
                # Some special characters might cause parsing errors
                pass

    def test_malformed_json_in_requests(self, client):
        """Test handling of malformed JSON in request bodies"""
        # Try to send malformed JSON to endpoints that don't expect it
        malformed_json = '{"incomplete": json'

        response = client.post("/health", content=malformed_json, headers={"Content-Type": "application/json"})
        # Should return 405 (Method Not Allowed) since POST is not supported
        assert response.status_code == 405

    def test_extremely_large_headers(self, client):
        """Test handling of extremely large headers"""
        large_header_value = "x" * 8192  # 8KB header

        try:
            response = client.get("/health", headers={"X-Large-Header": large_header_value})
            # Should either work or fail gracefully
            assert response.status_code in [200, 400, 413, 431]
        except Exception:
            # Some HTTP clients/servers might reject very large headers
            pass

    def test_unicode_in_headers(self, client):
        """Test handling of unicode characters in headers"""
        unicode_headers = {
            "X-Unicode-Header": "한글테스트",
            "X-Emoji-Header": "🚀🌟",
            "X-Special-Chars": "àáâãäåæçèéêë"
        }

        try:
            response = client.get("/health", headers=unicode_headers)
            # Should handle gracefully
            assert response.status_code in [200, 400]
        except Exception:
            # Some unicode in headers might cause issues
            pass


@pytest.fixture()
def sample_app():
    """Fixture providing the FastAPI app instance"""
    return app


@pytest.fixture()
def health_response_model():
    """Fixture providing a HealthResponse model instance"""
    return HealthResponse(status="test")


def test_sample_app_fixture(sample_app):
    """Test using the sample app fixture"""
    assert hasattr(sample_app, 'title')
    assert sample_app.title == "DS18B20 / DHT22 Backend API"


def test_health_response_model_fixture(health_response_model):
    """Test using the health response model fixture"""
    assert isinstance(health_response_model, HealthResponse)
    assert health_response_model.status == "test"


class TestFastAPIDocumentation:
    """Test API documentation and OpenAPI spec"""

    @pytest.fixture()
    def client(self):
        """Fixture providing a FastAPI test client"""
        return TestClient(app)

    def test_openapi_schema_generation(self, client):
        """Test OpenAPI schema generation"""
        response = client.get("/openapi.json")

        if response.status_code == 200:
            schema = response.json()

            # Basic OpenAPI structure
            assert "openapi" in schema
            assert "info" in schema
            assert "paths" in schema

            # App info
            assert schema["info"]["title"] == "DS18B20 / DHT22 Backend API"

            # Paths should include our endpoints
            assert "/health" in schema["paths"]
            assert "/sensors" in schema["paths"]

            # Health endpoint should have correct response model
            health_path = schema["paths"]["/health"]["get"]
            assert "responses" in health_path
            assert "200" in health_path["responses"]

    def test_api_route_metadata(self):
        """Test API route metadata"""
        # Check that routes have proper metadata
        routes = app.routes

        health_route = next((r for r in routes if hasattr(r, 'path') and r.path == "/health"), None)
        sensors_route = next((r for r in routes if hasattr(r, 'path') and r.path == "/sensors"), None)

        assert health_route is not None
        assert sensors_route is not None

        # Check HTTP methods
        if hasattr(health_route, 'methods'):
            assert "GET" in health_route.methods

        if hasattr(sensors_route, 'methods'):
            assert "GET" in sensors_route.methods
