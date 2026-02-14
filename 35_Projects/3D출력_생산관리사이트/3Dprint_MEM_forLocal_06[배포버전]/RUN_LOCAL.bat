@echo off
setlocal
cd /d %~dp0\40_Software

if not exist .venv (
  python -m venv .venv
)

.venv\Scripts\python.exe -m pip install --upgrade pip
.venv\Scripts\python.exe -m pip install -r requirements.txt

echo Starting local server on http://127.0.0.1:5000
.venv\Scripts\python.exe app.py
endlocal
