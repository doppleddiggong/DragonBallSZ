@echo off
setlocal
chcp 65001 >nul
set "SCRIPT_DIR=%~dp0"

rem Windows wrapper for daily DevLog generation (once per day)
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%run_generate_daily_devlog_once.ps1" -BackfillDays 30 -BuildSummary
set EC=%ERRORLEVEL%
if %EC% NEQ 0 (
  echo [RunDevLog] Failed with exit code %EC%.
) else (
  echo [RunDevLog] Completed successfully.
)
exit /b %EC%

