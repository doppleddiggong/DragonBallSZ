#!/usr/bin/env bash
set -euo pipefail

# Bash wrapper for Commit Message generator under Tools/commit_msg

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CMD_WIN_PATH="$SCRIPT_DIR/../RunGenCommitMsgPy.cmd"

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
  # Fallback: call Python generator directly from Tools root
  TOOLS_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
  OUT_TXT="$TOOLS_ROOT/commit_message.txt"
  if command -v py >/dev/null 2>&1; then
    py -3 "$TOOLS_ROOT/CommitMessage/gen_commit_msg.py" -MsgPath "$OUT_TXT" -AllowOverwrite -ToClipboard
  elif command -v python3 >/dev/null 2>&1; then
    python3 "$TOOLS_ROOT/CommitMessage/gen_commit_msg.py" -MsgPath "$OUT_TXT" -AllowOverwrite -ToClipboard
  else
    echo "No suitable runner found (powershell.exe/cmd.exe/py/python3)" >&2
    exit 1
  fi
fi

echo "[run_gen_commit_msg.sh] Done."
