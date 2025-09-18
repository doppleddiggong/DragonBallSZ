Param(
  [int]$BackfillDays = 0,
  [switch]$BuildSummary,
  [switch]$NoMerges
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-KstNow { return [DateTime]::UtcNow.AddHours(9) }

function Get-KstBoundsForToday09 {
  $nowKst = Get-KstNow
  $date = $nowKst.Date
  $today9 = [DateTimeOffset]::new([datetime]::new($date.Year,$date.Month,$date.Day,9,0,0), [TimeSpan]::FromHours(9))
  if ($nowKst -lt $today9.DateTime) {
    $date = $date.AddDays(-1)
    $today9 = [DateTimeOffset]::new([datetime]::new($date.Year,$date.Month,$date.Day,9,0,0), [TimeSpan]::FromHours(9))
  }
  $yest9 = $today9.AddDays(-1)
  return [PSCustomObject]@{ DateStr=$today9.Date.ToString('yyyy-MM-dd'); StartISO=$yest9.ToString('yyyy-MM-ddTHH:mm:sszzz'); EndISO=$today9.ToString('yyyy-MM-ddTHH:mm:sszzz') }
}

function Get-KstBoundsForDate09([datetime]$date) {
  $end = [DateTimeOffset]::new([datetime]::new($date.Year,$date.Month,$date.Day,9,0,0), [TimeSpan]::FromHours(9))
  $start = $end.AddDays(-1)
  return [PSCustomObject]@{ DateStr=$date.ToString('yyyy-MM-dd'); StartISO=$start.ToString('yyyy-MM-ddTHH:mm:sszzz'); EndISO=$end.ToString('yyyy-MM-ddTHH:mm:sszzz') }
}

function Invoke-GitLogRange([string]$SinceISO, [string]$UntilISO, [switch]$NoMerges) {
  $args = @('log'); if ($NoMerges) { $args += '--no-merges' }
  $args += @("--since=$SinceISO", "--until=$UntilISO", '--date=iso-strict', '--pretty=format:%H|%ad|%an|%s%n%b', '--numstat')
  $psi = New-Object System.Diagnostics.ProcessStartInfo -Property @{ FileName='git'; Arguments=($args -join ' '); RedirectStandardOutput=$true; RedirectStandardError=$true; UseShellExecute=$false; WorkingDirectory=(Get-Location).Path }
  $p = [System.Diagnostics.Process]::Start($psi); $out = $p.StandardOutput.ReadToEnd(); $p.WaitForExit(); if ($p.ExitCode -ne 0) { return '' }; return $out
}

function Parse-GitNumstat([string]$Text) {
  $commits = @(); $cur = $null
  foreach ($line in ($Text -split "`r?`n")) {
    if ($line -match '^(?<sha>[0-9a-f]{7,40})\|(?<date>[^|]+)\|(?<author>[^|]+)\|(?<sub>.*)$') {
      if ($cur) { $commits += $cur }
      $cur = [ordered]@{ sha=$Matches.sha; date=$Matches.date; author=$Matches.author; subject=$Matches.sub; body=@(); adds=0; dels=0; files=0 }
      continue
    }
    if ($line -match '^(?<adds>\d+|-)\t(?<dels>\d+|-)\t(?<path>.+)$') {
      if ($null -ne $cur) {
        $adds = 0; if ($Matches.adds -ne '-') { $adds = [int]$Matches.adds }
        $dels = 0; if ($Matches.dels -ne '-') { $dels = [int]$Matches.dels }
        $cur.adds += $adds; $cur.dels += $dels; $cur.files += 1
      }
      continue
    }
    if ($null -ne $cur) { $cur.body += $line }
  }
  if ($cur) { $commits += $cur }
  foreach ($c in $commits) {
    $bodyText = ($c.body -join "`n")
    $todos = [System.Text.RegularExpressions.Regex]::Matches($bodyText, '(?im)^\s*(?:todo:|@todo)\s*(.+)$') | ForEach-Object { $_.Groups[1].Value.Trim() }
    $c['todos'] = @($todos)
    $c['breaking'] = [bool]([System.Text.RegularExpressions.Regex]::IsMatch($bodyText, '(?i)breaking change'))
  }
  return ,$commits
}

function Classify-Commit([string]$Subject) {
  $type=''; $scope=$null; $sub=$Subject
  $safeSubject = if ($null -ne $Subject -and $Subject -ne '') { $Subject } else { '' }
  $m = [System.Text.RegularExpressions.Regex]::Match($safeSubject, '^(?<type>[a-zA-Z]+)(\((?<scope>[^)]+)\))?:\s*(?<sub>.+)$')
  if ($m.Success) { $type=$m.Groups['type'].Value.ToLower(); $scope=$m.Groups['scope'].Value; $sub=$m.Groups['sub'].Value }
  $cl='Progress'
  if ($type) {
    if ($type -match '^(feat|perf|refactor|style|build|revert|docs|test|ci|fix)$') { $cl='Done' }
    elseif ($type -match '^(chore|wip)$') { $cl='Progress' }
  } else {
    if ($sub -match '(?i)\bfix|bug|hotfix\b') { $cl='Done' }
    elseif ($sub -match '(?i)\bwip|partial|temp\b') { $cl='Progress' }
  }
  return @($cl, $scope, $sub)
}

function Render-DayMD([string]$DateStr, $Commits, [string]$OutPath) {
  $addsT = (($Commits | ForEach-Object { $_.adds }) | Measure-Object -Sum).Sum; if (-not $addsT) { $addsT = 0 }
  $delsT = (($Commits | ForEach-Object { $_.dels }) | Measure-Object -Sum).Sum; if (-not $delsT) { $delsT = 0 }
  $filesT = (($Commits | ForEach-Object { $_.files }) | Measure-Object -Sum).Sum; if (-not $filesT) { $filesT = 0 }
  $done=@(); $prog=@(); $need=@(); $todos=@(); $details=@()
  foreach ($c in $Commits) {
    $res = Classify-Commit $c.subject; $cls=$res[0]; $scope=$res[1]; $clean=$res[2]
    $sha7 = $c.sha.Substring(0,7); $scopeTag = if ($scope) { "[$scope] " } else { '' }
    $line = "$scopeTag$clean ($sha7) (+$($c.adds)/-$($c.dels), $($c.files) files)"
    if ($cls -eq 'Done') { $done += $line } else { $prog += $line }
    if ($c.breaking) { $need += "NOTE: migration needed ($sha7)" }
    foreach ($t in $c.todos) { $todos += "- [ ] $t (from: $sha7)" }
    $bodyText = ($c.body -join "`n").Trim()
    if ($bodyText) {
      $details += "- $scopeTag$clean ($sha7) @$($c.author) @ $($c.date)"
      foreach ($bl in ($bodyText -split "`r?`n")) { $details += "  > $bl" }
      $details += ''
    }
  }

  $doneOut = if ($done.Count -gt 0) { $done } else { @('(none)') }
  $progOut = if ($prog.Count -gt 0) { $prog } else { @('(none)') }
  $needOut = if ($need.Count -gt 0) { $need } else { @('(none)') }
  $todosOut = if ($todos.Count -gt 0) { $todos } else { @('(none)') }
  $detailsOut = if ($details.Count -gt 0) { $details } else { @('(none)') }

  $lines = @(
    "# Daily DevLog $DateStr (KST 09:00 boundary)",
    "",
    "## Summary",
    "Commits: $($Commits.Count) | Changes: +$addsT / -$delsT | Files: $filesT",
    "",
    "## Highlights",
    "(summarize 2-5 high-impact changes)",
    "",
    "## Commit-based Work Log",
    "### Done",
    $doneOut,
    "",
    "### In Progress",
    $progOut,
    "",
    "### Needs Attention",
    $needOut,
    "",
    "### TODO (from commits)",
    $todosOut,
    "",
    "## Details",
    $detailsOut,
    "",
    "## Metrics (approx)",
    "Commits: $($Commits.Count)",
    "Adds: +$addsT / Dels: -$delsT",
    "Files: $filesT",
    "Done: $($done.Count) | Progress: $($prog.Count) | Needs: $($need.Count)",
    "Todos: $($todos.Count)"
  )

  $outDir = Split-Path -Parent $OutPath
  New-Item -ItemType Directory -Force -Path $outDir | Out-Null
  $lines | Out-File -FilePath $OutPath -Encoding UTF8

  $metrics = [ordered]@{ date=$DateStr; commits=$Commits.Count; adds=$addsT; dels=$delsT; files=$filesT; done=$done.Count; progress=$prog.Count; needsAttention=$need.Count; todos=$todos.Count }
  ($metrics | ConvertTo-Json -Depth 3 -Compress) | Out-File -FilePath (Join-Path $outDir ("{0}.metrics.json" -f $DateStr)) -Encoding UTF8
}

function Build-Last30Summary([string]$OutDir) {
  $now = (Get-KstNow).Date
  $texts = @()
  for ($i=0; $i -lt 30; $i++) {
    $d = $now.AddDays(-$i); $p = Join-Path $OutDir ("{0}.md" -f ($d.ToString('yyyy-MM-dd')))
    if (Test-Path $p) { $texts += (Get-Content -Raw -Path $p -ErrorAction SilentlyContinue) }
  }
  if (-not $texts.Count) { return }
  $todoCount = @(($texts -join "`n") | Select-String -Pattern '^- \[ \]').Count
  $lines = @(
    "## 30-Day Briefing $($now.ToString('yyyy-MM-dd'))",
    "",
    "### Overview",
    "- Daily files: $($texts.Count) (last 30 days)",
    "- TODO outstanding (est.): $todoCount",
    "",
    "### Suggested Focus",
    "1) Prioritize open TODOs",
    "2) Review in-progress heavy days",
    "3) Improve Done/Progress ratio"
  )
  $out = Join-Path $OutDir '_Last30Summary.md'
  $lines | Out-File -FilePath $out -Encoding UTF8
}

# Main
$root = (Get-Location).Path
$outDir = Join-Path $root 'Documents/DevLog'

$b = Get-KstBoundsForToday09
$todayPath = Join-Path $outDir ("{0}.md" -f $b.DateStr)
if (-not (Test-Path $todayPath)) {
  $text = Invoke-GitLogRange -SinceISO $b.StartISO -UntilISO $b.EndISO -NoMerges:$NoMerges
  $commits = Parse-GitNumstat $text
  Render-DayMD -DateStr $b.DateStr -Commits $commits -OutPath $todayPath
}

if ($BackfillDays -gt 0) {
  $now = (Get-KstNow).Date
  for ($i=1; $i -le $BackfillDays; $i++) {
    $d = $now.AddDays(-$i)
    $bb = Get-KstBoundsForDate09 -date $d
    $outPath = Join-Path $outDir ("{0}.md" -f $bb.DateStr)
    if (Test-Path $outPath) { continue }
    $text = Invoke-GitLogRange -SinceISO $bb.StartISO -UntilISO $bb.EndISO -NoMerges:$NoMerges
    $commits = Parse-GitNumstat $text
    Render-DayMD -DateStr $bb.DateStr -Commits $commits -OutPath $outPath
  }
}

if ($BuildSummary) { Build-Last30Summary -OutDir $outDir }

Write-Output "[DevLog-PS] Generation complete."
