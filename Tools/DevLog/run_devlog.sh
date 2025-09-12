#!/usr/bin/env bash
set -euo pipefail

# Bash wrapper to run the Windows DevLog generator

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CMD_WIN_PATH="$SCRIPT_DIR/../RunDevLogPy.cmd"

# Convert to Windows path for PowerShell/cmd
if command -v cygpath >/dev/null 2>&1; then
  CMD_WIN_PATH="$(cygpath -w "$CMD_WIN_PATH")"
elif command -v wslpath >/dev/null 2>&1; then
  CMD_WIN_PATH="$(wslpath -w "$CMD_WIN_PATH")"
fi

if command -v powershell.exe >/dev/null 2>&1; then
  powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "& \"$CMD_WIN_PATH\""
elif command -v cmd.exe >/dev/null 2>&1; then
  cmd.exe /c "$CMD_WIN_PATH"
else
  # Fallback to python
  if command -v py >/dev/null 2>&1; then
    py -3 "$SCRIPT_DIR/run_generate_daily_devlog_once.py" -BackfillDays 30 -BuildSummary
  elif command -v python3 >/dev/null 2>&1; then
    python3 "$SCRIPT_DIR/run_generate_daily_devlog_once.py" -BackfillDays 30 -BuildSummary
  else
    echo "No suitable runner found (powershell.exe/cmd.exe/py/python3)" >&2
    exit 1
  fi
fi

echo "[run_devlog.sh] Done."
