Param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Assert-True([bool]$Cond, [string]$Msg) {
    if (-not $Cond) { throw "ASSERTION FAILED: $Msg" }
}

$Root = Join-Path $PSScriptRoot '..' | Resolve-Path | Select-Object -ExpandProperty Path
$TestRoot = Join-Path $Root 'Saved/TestRepo_AgentQA'
if (Test-Path $TestRoot) { Remove-Item -Recurse -Force $TestRoot }
New-Item -ItemType Directory -Force -Path $TestRoot | Out-Null

$Script = Join-Path $Root 'save_agent_qa.ps1'
$User = 'Tester'
$Tags = 'DragonBallSZ;Build;Decision'
$Q = 'DevLog 자동 생성의 Windows 통합 방안 합의'
$A = 'PreBuildSteps에 PowerShell 래퍼 호출, 일일/30일 로그 생성 및 중복 방지.'

try {
    & $Script -User $User -Tags $Tags -Question $Q -Answer $A -Root $TestRoot 2>&1 | Out-Null
} catch {
    Write-Output "[TEST-FAIL-EXPECTED] $_"
    exit 1
}

$today = (Get-Date).ToString('yyyy-MM-dd')
$MdPath = Join-Path $TestRoot ("Documents/AgentQA/$today.md")
$JsonlPath = Join-Path $TestRoot 'Documents/AgentQA/qa_log.jsonl'

Assert-True (Test-Path $MdPath) "Markdown file not created: $MdPath"
Assert-True (Test-Path $JsonlPath) "qa_log.jsonl not created: $JsonlPath"

$md = Get-Content $MdPath -Raw
Assert-True ($md -match '# Agent Q&A') 'Markdown missing header'
Assert-True ($md -match 'User:\s*Tester') 'Markdown missing user line'

$line = Get-Content $JsonlPath | Select-Object -Last 1
Assert-True ($line -match '"user"\s*:\s*"Tester"') 'JSONL missing user'
Assert-True ($line -match '"tags"') 'JSONL missing tags'
Assert-True ($line -match '"question"') 'JSONL missing question'
Assert-True ($line -match '"answer"') 'JSONL missing answer'

Write-Output "[TEST-PASS] AgentQA saver working"
exit 0
