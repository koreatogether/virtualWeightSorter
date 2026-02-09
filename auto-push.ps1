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

$script:pending = $false
Write-Log "auto-push started"
$script:pending = $false
$script:timer = New-Object Timers.Timer
$script:timer.Interval = $DebounceSeconds * 1000
$script:timer.AutoReset = $false
$script:timer.add_Elapsed({
  if (-not $script:pending) { return }
  $script:pending = $false

  Set-Location $Repo

  git add -A
  $status = git status --porcelain
  if (-not $status) { return }

  git commit -m "auto: update" | Out-Null

  $hasRemote = git remote
  if (-not $hasRemote) { return }

  for ($i = 1; $i -le $PushRetryCount; $i++) {
    git push | Out-Null
    if ($LASTEXITCODE -eq 0) {
      Write-Log "push success"
      break
    }
    Write-Log "push failed (attempt $i/$PushRetryCount), retrying in $PushRetryDelaySeconds s"
    Start-Sleep -Seconds $PushRetryDelaySeconds
  }
})

$watcher = New-Object IO.FileSystemWatcher $Repo, "*"
$watcher.IncludeSubdirectories = $true
$watcher.EnableRaisingEvents = $true

$action = {
  if ($Event.SourceEventArgs.FullPath -match "\\\.git\\") { return }
  $script:pending = $true
  $script:timer.Stop()
  $script:timer.Start()
}

Register-ObjectEvent $watcher Changed -Action $action | Out-Null
Register-ObjectEvent $watcher Created -Action $action | Out-Null
Register-ObjectEvent $watcher Deleted -Action $action | Out-Null
Register-ObjectEvent $watcher Renamed -Action $action | Out-Null

Write-Log "watching $Repo"
while ($true) { Start-Sleep -Seconds 1 }
