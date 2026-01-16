DS18B20 / DHT22 Monitoring - Development Notes and Integration Plan

Goal
- Reuse the Arduino simulator, serial communication layer, and dashboard code from
  `e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb` and integrate them into
  `e:\project\05_P_ds18b20_monitoring`.
- Use a Python virtual environment and run backend APIs with Uvicorn (ASGI).

Checklist (high level)
1. Create virtual environment and install packages (`requirements.txt`).
2. Copy simulator and serial modules from the DS18b20 project into `src/python/simulator` and `src/python/backend`.
3. Copy dashboard assets (JS/CSS/templates) into `src/python/dashboard/static` and `templates`.
4. Adapt `app.py` (Dash) and backend entrypoint to new paths. Run backend APIs with `uvicorn` and run Dash either standalone or mounted behind FastAPI.

Files in the source project to copy (recommended)
- e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\app.py
- e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\core\arduino_manager.py
- e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\core\serial_json_communication.py
- e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\core\port_manager.py
- e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\core\shared_callbacks.py
- e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\assets\ (copy relevant JS/CSS)

Quick copy commands (PowerShell)
# from project root (e:\project\05_P_ds18b20_monitoring)
# copy core modules
Copy-Item -Path "e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\core\arduino_manager.py" -Destination ".\src\python\backend\arduino_manager.py" -Force
Copy-Item -Path "e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\core\serial_json_communication.py" -Destination ".\src\python\backend\serial_json_communication.py" -Force
Copy-Item -Path "e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\core\port_manager.py" -Destination ".\src\python\backend\port_manager.py" -Force

# copy app and assets
Copy-Item -Path "e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\app.py" -Destination ".\src\python\dashboard\app.py" -Force
Copy-Item -Path "e:\project\01_P_DS18b20\DS18b20_Arduino_DashWeb\src_dash\assets\*" -Destination ".\src\python\dashboard\static\" -Recurse -Force

Integration notes and options
- Option A (simple): Run Dash app directly (app.run_server). Use this for development and local testing.
- Option B (recommended for unified dev server): Keep Dash for UI and create a small FastAPI app for APIs and background tasks. Start FastAPI with Uvicorn and mount or proxy Dash. Example strategies are documented below.

Running with Uvicorn + FastAPI
- Create `src/python/backend/main.py` as FastAPI entry. This file can import and mount Dash as a WSGI/ASGI app or serve APIs that the Dash frontend calls via fetch/WS.
- Start with:
    & .\.venv\Scripts\Activate.ps1
    uvicorn src.python.backend.main:app --reload --host 127.0.0.1 --port 8000

Notes on Dash + ASGI
- Dash historically is WSGI (Flask). If you want a single ASGI server, you can:
  - Wrap the Dash Flask server with `asgiref.wsgi.WsgiToAsgi` and mount under FastAPI.
  - Or run Dash on its own port and reverse-proxy / proxy requests in production.

Next steps I can take for you
- Copy the minimal modules (arduino_manager + serial_json_communication + app.py + assets) into `05_P_ds18b20_monitoring` and adapt imports. I can do that now if you want.

