def test_app_importable():
    # Import by package path to ensure src/python is a package
    from src.python import app

    assert hasattr(app, "app") or hasattr(app, "layout") or getattr(app, "__name__", None)
