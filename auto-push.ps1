param(
  [string]$Repo = $PWD.Path,
  [int]$DebounceSeconds = 5,
  [int]$PushRetryCount = 5,
  [int]$PushRetryDelaySeconds = 10,
  [string]$LogPath = "$env:LOCALAPPDATA\AutoGitPush\auto-push.log",
  [int]$LogMaxBytes = 1MB,
  [int]$LogKeepFiles = 5
)

Set-Location $Repo
Write-Host "Script started for repo: $Repo"

function Write-Log {
  param([string]$Message)
  $ts = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
  $logDir = Split-Path -Parent $LogPath
  if (-not (Test-Path $logDir)) {
    New-Item -ItemType Directory -Path $logDir -Force | Out-Null
  }
  if (Test-Path $LogPath) {
    $size = (Get-Item $LogPath).Length
    if ($size -ge $LogMaxBytes) {
      for ($i = $LogKeepFiles - 1; $i -ge 1; $i--) {
        $src = "$LogPath.$i"
        $dst = "$LogPath." + ($i + 1)
        if (Test-Path $src) { Move-Item $src $dst -Force }
      }
      Move-Item $LogPath "$LogPath.1" -Force
    }
  }
  Add-Content -Path $LogPath -Value "[$ts] $Message"
}

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
  Write-Log "git not found in PATH"
  exit 1
}

$repoBytes = [Text.Encoding]::UTF8.GetBytes($Repo)
$hash = [BitConverter]::ToString([Security.Cryptography.SHA256]::Create().ComputeHash($repoBytes)).Replace("-", "")
$mutexName = "AutoGitPush_$hash"
$mutex = New-Object System.Threading.Mutex($false, $mutexName)
if (-not $mutex.WaitOne(0)) {
  Write-Log "another instance is already running; exiting"
  exit 0
}

Write-Log "auto-push started (polling)"
Write-Log "watching $Repo"

$lastSeenChangesAt = $null

while ($true) {
  Start-Sleep -Seconds 2
  Set-Location $Repo

  $status = git status --porcelain
  if (-not $status) {
    $lastSeenChangesAt = $null
    continue
  }

  if (-not $lastSeenChangesAt) {
    $lastSeenChangesAt = Get-Date
    Write-Log "changes detected (poll)"
    continue
  }

  $elapsed = (Get-Date) - $lastSeenChangesAt
  if ($elapsed.TotalSeconds -lt $DebounceSeconds) { continue }

  # VS Code 알림 메시지 상자 띄우기 (VBScript 활용하여 팝업 생성)
  $wshell = New-Object -ComObject WScript.Shell
  $msg = "변경사항이 감지되었습니다. 커밋하고 푸시하시겠습니까?"
  $title = "Auto-Git-Push 승인"
  # 4 = Yes/No 버튼, 32 = Question 아이콘, 256 = 두 번째 버튼 기본값
  $response = $wshell.Popup($msg, 0, $title, 4 + 32)

  if ($response -ne 6) {
    # 6은 '예(Yes)'를 클릭했을 때의 값
    Write-Log "User declined push. Waiting for next changes."
    $lastSeenChangesAt = $null
    continue
  }

  git add -A
  $status2 = git status --porcelain
  if (-not $status2) {
    $lastSeenChangesAt = $null
    continue
  }

  git commit -m "auto: update" | Out-Null
  Write-Log "commit created"

  $hasRemote = git remote
  if ($hasRemote) {
    for ($i = 1; $i -le $PushRetryCount; $i++) {
      git push | Out-Null
      if ($LASTEXITCODE -eq 0) {
        Write-Log "push success"
        break
      }
      Write-Log "push failed (attempt $i/$PushRetryCount), retrying in $PushRetryDelaySeconds s"
      Start-Sleep -Seconds $PushRetryDelaySeconds
    }
  }

  $lastSeenChangesAt = $null
}
