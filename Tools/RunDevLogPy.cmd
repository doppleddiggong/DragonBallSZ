@echo off
setlocal
chcp 65001 >nul
set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"

where python >nul 2>nul
if %ERRORLEVEL%==0 (
  python "%SCRIPT_DIR%DevLog\run_generate_daily_devlog_once.py" -BackfillDays 30 -BuildSummary
) else (
  where py >nul 2>nul
  if %ERRORLEVEL%==0 (
    py -3 "%SCRIPT_DIR%DevLog\run_generate_daily_devlog_once.py" -BackfillDays 30 -BuildSummary
  ) else (
    echo [RunDevLogPy] Python not found.
    pause & exit /b 1
  )
)

if not "%ERRORLEVEL%"=="0" (
  echo [RunDevLogPy] Failed with exit code %ERRORLEVEL%.
) else (
  echo [RunDevLogPy] Completed successfully.
)
pause
exit /b %ERRORLEVEL%
