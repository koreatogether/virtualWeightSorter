param(
  [string]$Repo = "c:\project\private_projects",
  [int]$DebounceSeconds = 5,
  [int]$PushRetryCount = 5,
  [int]$PushRetryDelaySeconds = 10,
  [string]$LogPath = "c:\project\private_projects\auto-push.log",
  [int]$LogMaxBytes = 1MB,
  [int]$LogKeepFiles = 5
)

Set-Location $Repo

function Write-Log {
  param([string]$Message)
  $ts = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
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
