# UV 가상환경 자동 활성화 스크립트 (PowerShell)
Set-Location $PSScriptRoot
Write-Host "Activating UV virtual environment..." -ForegroundColor Green

# UV 가상환경 활성화
& ".venv\Scripts\Activate.ps1"

Write-Host "UV virtual environment activated!" -ForegroundColor Yellow
Write-Host "Project root: $(Get-Location)" -ForegroundColor Cyan
Write-Host "Python version:" -ForegroundColor Cyan
python --version