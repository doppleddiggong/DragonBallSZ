@echo off
setlocal
chcp 65001 >nul

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"

echo [SetupProject] Step 1/3: Ensuring Python is installed
call "%SCRIPT_DIR%InstallPython.cmd"
if not "%ERRORLEVEL%"=="0" (
  echo [SetupProject] Python install step did not succeed. Please install manually and re-run.
  pause & exit /b 1
)

echo [SetupProject] Step 2/3: Installing Git hook (prepare-commit-msg)
where python >nul 2>nul
if %ERRORLEVEL%==0 (
  python "%SCRIPT_DIR%install_commit_hook.py"
) else (
  where py >nul 2>nul
  if %ERRORLEVEL%==0 (
    py -3 "%SCRIPT_DIR%install_commit_hook.py"
  ) else (
    echo [SetupProject] Python not found; skipping hook install.
  )
)

echo [SetupProject] Step 3/3: Generating DevLog once
call "%SCRIPT_DIR%RunDevLogPy.cmd"

echo [SetupProject] Complete.
pause
exit /b 0

