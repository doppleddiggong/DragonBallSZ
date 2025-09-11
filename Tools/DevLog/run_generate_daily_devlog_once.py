#!/usr/bin/env python3
import os, sys, subprocess, datetime

def main():
    # args: -BackfillDays N -BuildSummary
    args = sys.argv[1:]
    backfill = 30
    build_summary = True
    i=0
    while i < len(args):
        a = args[i].lower()
        if a == '-backfilldays' and i+1 < len(args):
            try: backfill = int(args[i+1])
            except: backfill = backfill
            i += 2; continue
        if a == '-buildsummary':
            build_summary = True; i += 1; continue
        i += 1

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.abspath(os.path.join(script_dir, '..'))
    flag_dir = os.path.join(project_dir, 'Saved', 'DevLog')
    os.makedirs(flag_dir, exist_ok=True)
    flag_file = os.path.join(flag_dir, 'last_generate_devlog.txt')
    today = datetime.date.today().strftime('%Y-%m-%d')

    if os.path.isfile(flag_file):
        try:
            last = open(flag_file,'r',encoding='utf-8').read().strip()
        except Exception:
            last = ''
        if last == today:
            print(f"[DevLog] Already ran today ({today}). Skipping.")
            return 0

    gen = os.path.join(script_dir, 'generate_daily_devlog_bi.py')
    if not os.path.isfile(gen):
        print(f"[DevLog] Generator not found: {gen}")
        return 1
    print(f"[DevLog] Running generator: -BackfillDays {backfill} -BuildSummary:{build_summary}")
    cmd = [sys.executable or 'python', gen, '-BackfillDays', str(backfill)]
    if build_summary:
        cmd.append('-BuildSummary')
    cp = subprocess.run(cmd, cwd=project_dir)
    if cp.returncode != 0:
        print("[DevLog] Failed to generate.")
        return cp.returncode
    with open(flag_file,'w',encoding='utf-8') as f:
        f.write(today)
    print(f"[DevLog] Done. Flag updated: {today}")
    return 0

if __name__ == '__main__':
    sys.exit(main())

