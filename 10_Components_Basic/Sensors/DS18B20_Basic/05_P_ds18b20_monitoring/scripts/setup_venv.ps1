# PowerShell script: create and activate virtual environment, then install requirements
# Usage: Run in PowerShell from the project root (e:\project\05_P_ds18b20_monitoring) with Administrator privileges if needed.

python -m venv .venv
# Activate venv (PowerShell)
& .\.venv\Scripts\Activate.ps1
# Upgrade pip and install requirements
python -m pip install --upgrade pip
python -m pip install -r requirements.txt

Write-Host "\nVirtual environment ready. Activate with: & .\.venv\Scripts\Activate.ps1" -ForegroundColor Green
