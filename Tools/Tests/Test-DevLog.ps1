Param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-KstNow {
    # Returns current time in KST
    return [DateTimeOffset]::UtcNow.ToOffset([TimeSpan]::FromHours(9))
}

function Get-KstWindow {
    $nowKst = Get-KstNow
    $today = $nowKst.Date
    $today9 = [DateTimeOffset]::new($today.Year,$today.Month,$today.Day,9,0,0,[TimeSpan]::FromHours(9))
    if ($nowKst -lt $today9) {
        $today = $today.AddDays(-1)
        $today9 = [DateTimeOffset]::new($today.Year,$today.Month,$today.Day,9,0,0,[TimeSpan]::FromHours(9))
    }
    $yest9 = $today9.AddDays(-1)
    return [PSCustomObject]@{ Date=$today; Start=$yest9; End=$today9 }
}

function New-TestRepo([string]$Path) {
    if (Test-Path $Path) { Remove-Item -Recurse -Force $Path }
    New-Item -ItemType Directory -Force -Path $Path | Out-Null
    pushd $Path
    git init | Out-Null
    git config user.name "TestUser" | Out-Null
    git config user.email "test@example.com" | Out-Null
    'README' | Set-Content README.md -Encoding UTF8
    git add . | Out-Null
    git commit -m "chore: init" | Out-Null
    popd
}

function New-WindowCommit([string]$Repo, [string]$Message) {
    $win = Get-KstWindow
    # pick mid-window time (yesterday 21:00 KST relative to window start date)
    $start = $win.Start.ToOffset([TimeSpan]::FromHours(9))
    $targetKst = [DateTimeOffset]::new($start.Year,$start.Month,$start.Day,21,0,0,[TimeSpan]::FromHours(9))
    $utc = $targetKst.ToUniversalTime()
    $iso = $utc.ToString("yyyy-MM-ddTHH:mm:ssK")
    pushd $Repo
    'x=1' | Set-Content sample.txt -Encoding UTF8
    git add sample.txt | Out-Null
    $env:GIT_AUTHOR_DATE = $iso
    $env:GIT_COMMITTER_DATE = $iso
    git commit -m $Message -m "TODO: add e2e tests" | Out-Null
    Remove-Item Env:GIT_AUTHOR_DATE -ErrorAction SilentlyContinue
    Remove-Item Env:GIT_COMMITTER_DATE -ErrorAction SilentlyContinue
    popd
}

function Assert-True([bool]$Cond, [string]$Msg) {
    if (-not $Cond) { throw "ASSERTION FAILED: $Msg" }
}

# Arrange
$Root = Join-Path $PSScriptRoot '..' | Resolve-Path | Select-Object -ExpandProperty Path
$TestRepo = Join-Path $Root 'Saved/TestRepo_DevLog'
New-TestRepo -Path $TestRepo
New-WindowCommit -Repo $TestRepo -Message 'feat(Character): Add Dash system core'

# Act (expected to fail before implementation)
$Script = Join-Path $Root 'run_generate_daily_devlog_once.ps1'
try {
    $out = & $Script -RepoPath $TestRepo -BackfillDays 0 -BuildSummary -Force 2>&1
} catch {
    Write-Output "[TEST-FAIL-EXPECTED] $_"
    exit 1
}

# Assert
$win = Get-KstWindow
$dateStr = $win.Date.ToString('yyyy-MM-dd')
$OutDir = Join-Path $TestRepo 'Documents/DevLog'
$DailyPath = Join-Path $OutDir ("$dateStr.md")
Assert-True (Test-Path $DailyPath) "Daily DevLog file not created: $DailyPath"
$content = Get-Content $DailyPath -Raw -ErrorAction Stop
Assert-True ($content -match 'Daily DevLog') 'Missing daily devlog title'
Assert-True ($content -match 'Add Dash system core') 'Missing commit subject in devlog'
Assert-True ($content -match '\- \[ \] .*\(출처:') 'Missing TODO extraction section'
Assert-True ($content -match '## Commit Details') 'Missing Commit Details section'
Assert-True ($content -match 'TODO: add e2e tests') 'Missing commit body content in details'

# Structured log presence
Assert-True ($out -match '"Operation":"GenerateDailyDevLog"') 'Missing structured log output'

$SummaryPath = Join-Path $OutDir '_Last30Summary.md'
Assert-True (Test-Path $SummaryPath) 'Missing _Last30Summary.md summary file'

Write-Output "[TEST-PASS] DevLog generation wrapper working"
exit 0
