Param(
  [int]$BackfillDays = 30,
  [switch]$BuildSummary,
  [string]$RepoPath,
  [switch]$Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Write-StructuredLog {
  Param(
    [string]$Operation,
    [int]$Attempt,
    [int]$MaxAttempts,
    [TimeSpan]$Duration,
    [string]$Outcome,
    [string]$ErrorType,
    [string]$Message
  )
  $log = [PSCustomObject]@{
    CorrelationId = [guid]::NewGuid().ToString()
    Operation     = $Operation
    Attempt       = $Attempt
    MaxAttempts   = $MaxAttempts
    DurationMs    = [int]$Duration.TotalMilliseconds
    Outcome       = $Outcome
    ErrorType     = $ErrorType
    Message       = $Message
  }
  $log | ConvertTo-Json -Compress
}

$op = 'GenerateDailyDevLog'
$sw = [System.Diagnostics.Stopwatch]::StartNew()
try {
  $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
  $root = if ([string]::IsNullOrWhiteSpace($RepoPath)) { Split-Path -Parent $scriptDir } else { Resolve-Path $RepoPath | Select-Object -ExpandProperty Path }

  # Always use generator from current project Tools directory, but run in -RepoPath
  $projRootForGenerator = Split-Path -Parent $scriptDir
  $pyRunner = if ($Force) {
    # call generator directly to bypass once-per-day flag
    Join-Path $projRootForGenerator 'Tools/DevLog/generate_daily_devlog_bi.py'
  } else {
    Join-Path $projRootForGenerator 'Tools/DevLog/run_generate_daily_devlog_once.py'
  }
  if (-not (Test-Path $pyRunner)) { throw "Generator not found: $pyRunner" }

  $exe = $null
  $python = (Get-Command python -ErrorAction SilentlyContinue)
  if ($python) { $exe = $python.Source } else {
    $py = (Get-Command py -ErrorAction SilentlyContinue)
    if ($py) { $exe = $py.Source }
  }

  if (-not $exe) {
    # Fallback to PowerShell generator (no Python available)
    $flagDir = Join-Path $root 'Saved/DevLog'
    New-Item -ItemType Directory -Force -Path $flagDir | Out-Null
    $flagFile = Join-Path $flagDir 'last_generate_devlog.txt'
    $kst = [DateTime]::UtcNow.AddHours(9)
    $today = $kst.ToString('yyyy-MM-dd')
    if ((Test-Path $flagFile) -and ((Get-Content -Path $flagFile -ErrorAction SilentlyContinue).Trim() -eq $today) -and (-not $Force)) {
      $sw.Stop()
      Write-StructuredLog -Operation $op -Attempt 1 -MaxAttempts 1 -Duration $sw.Elapsed -Outcome 'Success' -ErrorType '' -Message 'DevLog already generated today (fallback PS).'
      exit 0
    }

    $psGen = Join-Path $projRootForGenerator 'Tools/DevLog/generate_daily_devlog_ps.ps1'
    if (-not (Test-Path $psGen)) { throw "Fallback generator not found: $psGen" }
    $psArgs = @('-File', $psGen, '-BackfillDays', "$BackfillDays")
    if ($BuildSummary) { $psArgs += '-BuildSummary' }
    $out = powershell -NoProfile @psArgs 2>&1
    if ($LASTEXITCODE -ne 0) { throw "Fallback generator failed: $out" }
    Set-Content -Path $flagFile -Value $today -Encoding UTF8
    $sw.Stop()
    Write-StructuredLog -Operation $op -Attempt 1 -MaxAttempts 1 -Duration $sw.Elapsed -Outcome 'Success' -ErrorType '' -Message 'DevLog generated via PowerShell fallback.'
    if ($out) { Write-Output $out }
    exit 0
  }

  $args = @($pyRunner,'-BackfillDays',"$BackfillDays")
  if ($BuildSummary) { $args += '-BuildSummary' }

  $pinfo = New-Object System.Diagnostics.ProcessStartInfo
  $pinfo.FileName = $exe
  if ($exe.ToLower().EndsWith('py.exe')) { $pinfo.Arguments = '-3 ' + ($args -join ' ') } else { $pinfo.Arguments = ($args -join ' ') }
  $pinfo.WorkingDirectory = $root
  $pinfo.RedirectStandardOutput = $true
  $pinfo.RedirectStandardError = $true
  $pinfo.UseShellExecute = $false

  $proc = [System.Diagnostics.Process]::Start($pinfo)
  $stdout = $proc.StandardOutput.ReadToEnd()
  $stderr = $proc.StandardError.ReadToEnd()
  $proc.WaitForExit()
  $code = $proc.ExitCode

  if ($code -ne 0) {
    # Attempt PowerShell fallback if Python invocation failed
    try {
      $psGen = Join-Path $projRootForGenerator 'Tools/DevLog/generate_daily_devlog_ps.ps1'
      if (-not (Test-Path $psGen)) { throw "Fallback generator not found: $psGen" }
      $flagDir = Join-Path $root 'Saved/DevLog'
      New-Item -ItemType Directory -Force -Path $flagDir | Out-Null
      $flagFile = Join-Path $flagDir 'last_generate_devlog.txt'
      $kst = [DateTime]::UtcNow.AddHours(9)
      $today = $kst.ToString('yyyy-MM-dd')
      if (-not $Force) {
        if ((Test-Path $flagFile) -and ((Get-Content -Path $flagFile -ErrorAction SilentlyContinue).Trim() -eq $today)) {
          $sw.Stop()
          Write-StructuredLog -Operation $op -Attempt 1 -MaxAttempts 1 -Duration $sw.Elapsed -Outcome 'Success' -ErrorType '' -Message 'DevLog already generated today (fallback PS after Python fail).'
          exit 0
        }
      }
      $psArgs = @('-File', $psGen, '-BackfillDays', "$BackfillDays")
      if ($BuildSummary) { $psArgs += '-BuildSummary' }
      $out = powershell -NoProfile @psArgs 2>&1
      if ($LASTEXITCODE -ne 0) { throw "Fallback generator failed: $out" }
      Set-Content -Path $flagFile -Value $today -Encoding UTF8
      $sw.Stop()
      Write-StructuredLog -Operation $op -Attempt 1 -MaxAttempts 1 -Duration $sw.Elapsed -Outcome 'Success' -ErrorType '' -Message 'DevLog generated via PowerShell fallback (after Python fail).'
      if ($out) { Write-Output $out }
      exit 0
    }
    catch {
      throw "Generator failed ($code): $stderr $stdout"
    }
  }

  $sw.Stop()
  Write-StructuredLog -Operation $op -Attempt 1 -MaxAttempts 1 -Duration $sw.Elapsed -Outcome 'Success' -ErrorType '' -Message 'DevLog generated (once per day).'
  if ($stdout) { Write-Output $stdout }
  exit 0
}
catch {
  $sw.Stop()
  Write-StructuredLog -Operation $op -Attempt 1 -MaxAttempts 3 -Duration $sw.Elapsed -Outcome 'Fail' -ErrorType ($_.FullyQualifiedErrorId) -Message ($_.Exception.Message) | Write-Output
  exit 1
}
