$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$swDir = Join-Path $root "40_Software"
Set-Location $swDir

if (!(Test-Path ".venv")) {
  python -m venv .venv
}

& ".\.venv\Scripts\python.exe" -m pip install --upgrade pip
& ".\.venv\Scripts\python.exe" -m pip install -r requirements.txt

Write-Host "Starting local server on http://127.0.0.1:5000"
& ".\.venv\Scripts\python.exe" app.py
