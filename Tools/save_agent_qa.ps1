Param(
  [Parameter(Mandatory=$true)][string]$User,
  [Parameter(Mandatory=$true)][string]$Tags,
  [Parameter(Mandatory=$true)][string]$Question,
  [Parameter(Mandatory=$true)][string]$Answer,
  [string]$Root
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

$op = 'SaveAgentQA'
$sw = [System.Diagnostics.Stopwatch]::StartNew()
try {
  $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
  $root = if ([string]::IsNullOrWhiteSpace($Root)) { Split-Path -Parent $scriptDir } else { Resolve-Path $Root | Select-Object -ExpandProperty Path }

  $kstNow = [DateTimeOffset]::UtcNow.ToOffset([TimeSpan]::FromHours(9))
  $dateStr = $kstNow.ToString('yyyy-MM-dd')
  $qaDir = Join-Path $root 'Documents/AgentQA'
  New-Item -ItemType Directory -Force -Path $qaDir | Out-Null

  $mdPath = Join-Path $qaDir ("$dateStr.md")
  $jsonlPath = Join-Path $qaDir 'qa_log.jsonl'

  $md = @(
    "# Agent Q&A — $dateStr",
    "",
    "- User: $User",
    "- Tags: $Tags",
    "- Time(KST): " + $kstNow.ToString('HH:mm:ss'),
    "",
    "## Q",
    $Question,
    "",
    "## A",
    $Answer,
    ""
  ) -join "`r`n"
  Add-Content -Path $mdPath -Value $md -Encoding UTF8

  $ev = [PSCustomObject]@{
    tsKst = $kstNow.ToString('o')
    user = $User
    tags = $Tags
    question = $Question
    answer = $Answer
  }
  $json = $ev | ConvertTo-Json -Compress
  Add-Content -Path $jsonlPath -Value $json -Encoding UTF8

  $sw.Stop()
  Write-StructuredLog -Operation $op -Attempt 1 -MaxAttempts 1 -Duration $sw.Elapsed -Outcome 'Success' -ErrorType '' -Message 'Agent QA saved.'
  exit 0
}
catch {
  $sw.Stop()
  Write-StructuredLog -Operation $op -Attempt 1 -MaxAttempts 3 -Duration $sw.Elapsed -Outcome 'Fail' -ErrorType ($_.FullyQualifiedErrorId) -Message ($_.Exception.Message) | Write-Output
  exit 1
}

