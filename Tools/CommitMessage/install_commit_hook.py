#!/usr/bin/env python3
import os, sys

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.abspath(os.path.join(script_dir, '..'))
    hooks = os.path.join(repo_root, '.git', 'hooks')
    if not os.path.isdir(hooks):
        print(f".git/hooks not found: {hooks}")
        return 1
    hook_path = os.path.join(hooks, 'prepare-commit-msg')
    hook = """#!/bin/sh
# Auto-generate Conventional Commit message

MSGFILE="$1"
SOURCE="$2"

if [ -s "$MSGFILE" ]; then exit 0; fi
if [ "$SOURCE" = "merge" ]; then exit 0; fi
if [ -n "$SKIP_AUTO_COMMIT_MSG" ]; then exit 0; fi

if command -v python >/dev/null 2>&1; then
  python "Tools/CommitMessage/gen_commit_msg.py" -MsgPath "$MSGFILE"
elif command -v py >/dev/null 2>&1; then
  py -3 "Tools/CommitMessage/gen_commit_msg.py" -MsgPath "$MSGFILE"
elif command -v pwsh >/dev/null 2>&1; then
  pwsh -NoProfile -ExecutionPolicy Bypass -File "Tools/generate_commit_message.ps1" -MsgPath "$MSGFILE"
else
  powershell -NoProfile -ExecutionPolicy Bypass -File "Tools/generate_commit_message.ps1" -MsgPath "$MSGFILE"
fi

exit 0
"""
    with open(hook_path, 'w', encoding='utf-8') as f:
        f.write(hook)
    print(f"Installed Git hook: {hook_path}")
    return 0

if __name__ == '__main__':
    sys.exit(main())
