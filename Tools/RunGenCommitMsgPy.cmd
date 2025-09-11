@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

rem Resolve paths
set "SCRIPT_DIR=%~dp0"
set "OUT_TXT=%SCRIPT_DIR%..\commit_message.txt"

rem Find repository root by scanning upwards for .git
set "REPO_ROOT="
for %%G in ("%SCRIPT_DIR%" "%SCRIPT_DIR%.." "%SCRIPT_DIR%..\.." "%SCRIPT_DIR%..\..\.." "%SCRIPT_DIR%..\..\..\..") do (
  if exist "%%~fG\.git" set "REPO_ROOT=%%~fG"
)
if not defined REPO_ROOT (
  rem Fallback: assume Tools\.. is repo root
  for %%I in ("%SCRIPT_DIR%..\..") do set "REPO_ROOT=%%~fI"
)
set "COMMIT_MSG=%REPO_ROOT%\.git\COMMIT_EDITMSG"

rem Git repo check
if not exist "%REPO_ROOT%\.git" goto :not_git

rem Ensure auto-generation is not skipped for this run
set SKIP_AUTO_COMMIT_MSG=

rem Resolve a working Python interpreter into RUNNER (values: "py -3" or "python")
set "RUNNER="
set "PYFIRST="

rem Try py -3
where py >nul 2>nul
if errorlevel 1 goto :check_python
py -3 --version >nul 2>nul
if errorlevel 1 goto :check_python
set "RUNNER=py -3"
goto :have_python

:check_python
for /f "delims=" %%P in ('where python 2^>nul') do if not defined PYFIRST set "PYFIRST=%%P"
if not defined PYFIRST goto :have_python
set "CANDIDATE=%PYFIRST%"
set "CANDIDATE_CLEAN=%CANDIDATE:\Microsoft\WindowsApps\python.exe=%"
if /I "%CANDIDATE_CLEAN%"=="%CANDIDATE%" set "RUNNER=python"

:have_python
if not defined RUNNER goto :no_python

rem Run script with selected interpreter
if "%RUNNER%"=="py -3" goto :use_py
if "%RUNNER%"=="python" goto :use_python
goto :no_python

:use_py
py -3 "%SCRIPT_DIR%CommitMessage\gen_commit_msg.py" -MsgPath "%OUT_TXT%" -AllowOverwrite -ToClipboard %*
set "LASTERR=%ERRORLEVEL%"
goto :after_run

:use_python
python "%SCRIPT_DIR%CommitMessage\gen_commit_msg.py" -MsgPath "%OUT_TXT%" -AllowOverwrite -ToClipboard %*
set "LASTERR=%ERRORLEVEL%"
goto :after_run

:after_run
if not "%LASTERR%"=="0" goto :py_failed

if not exist "%OUT_TXT%" goto :no_out

copy /Y "%OUT_TXT%" "%COMMIT_MSG%" >nul
echo [RunGenCommitMsgPy] Wrote: %OUT_TXT%
echo [RunGenCommitMsgPy] Updated: %COMMIT_MSG%
goto :done

:not_git
echo [RunGenCommitMsgPy] Not a git repo: %REPO_ROOT%
pause
exit /b 1

:no_python
echo [RunGenCommitMsgPy] Python not found or invalid (WindowsApps alias).
echo   - Try: %SCRIPT_DIR%InstallPython.cmd
echo   - Or install Python 3.11+ and enable the 'py' launcher.
pause
exit /b 1

:py_failed
echo [RunGenCommitMsgPy] Python script failed with exit code %LASTERR%.
pause
exit /b %LASTERR%

:no_out
echo [RunGenCommitMsgPy] Failed to create: %OUT_TXT%
pause
exit /b 1

:done
pause
exit /b 0
