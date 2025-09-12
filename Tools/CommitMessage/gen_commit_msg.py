#!/usr/bin/env python3
import os, sys, subprocess, re, json

def run_git(args, cwd):
    try:
        cp = subprocess.run(["git"] + args, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, encoding=sys.getfilesystemencoding() or 'utf-8', errors='replace')
        if cp.returncode != 0:
            return ""
        return cp.stdout
    except Exception:
        return ""

def repo_root(start_dir):
    out = run_git(["rev-parse", "--show-toplevel"], start_dir).strip()
    return out if out else start_dir

def staged_name_status(root):
    out = run_git(["diff", "--cached", "--name-status"], root)
    files = []
    for line in out.splitlines():
        m = re.match(r"^(?P<status>[ACDMRTU])\s+(?P<path>.+)$", line)
        if m:
            files.append({"status": m.group("status"), "path": m.group("path")})
    return files

def staged_numstat(root):
    out = run_git(["diff", "--cached", "--numstat"], root)
    stats = {}
    for line in out.splitlines():
        m = re.match(r"^(?P<adds>\d+|-)\t(?P<dels>\d+|-)\t(?P<path>.+)$", line)
        if m:
            adds = 0 if m.group("adds") == '-' else int(m.group("adds"))
            dels = 0 if m.group("dels") == '-' else int(m.group("dels"))
            stats[m.group("path")] = {"adds": adds, "dels": dels}
    return stats

def guess_scope(path):
    m = re.match(r"^Source/DragonBallSZ/([^/]+)/", path)
    if m: return m.group(1)
    m = re.match(r"^Source/CoffeeLibrary/([^/]+)/", path)
    if m: return f"lib-{m.group(1)}"
    if path.startswith("Plugins/"):
        parts = path.split('/')
        if len(parts) > 1: return f"plugin-{parts[1]}"
    if path.startswith("Content/"): return "content"
    if path.startswith("Config/"): return "config"
    if re.match(r"^Documents?/", path): return "docs"
    return "core"

def guess_type(files, branch):
    m = re.match(r"^(feat|fix|refactor|docs|build|ci|test|perf|chore)(/|$)", branch or "")
    if m: return m.group(1)
    if any(f["status"] == 'A' for f in files): return 'feat'
    if any(f["path"].startswith("Config/") for f in files): return 'chore'
    return 'chore'

def build_subject(commit_type, scope, adds, dels, count):
    prefix = f"{commit_type}({scope}): " if scope else f"{commit_type}: "
    summary = f"파일 {count}개 변경 (+{adds}/-{dels})"
    limit = 72
    avail = max(1, limit - len(prefix))
    if len(summary) > avail:
        summary = summary[:max(0, avail-3)] + "..."
    return prefix + summary

def analyze_changes(root):
    out = run_git(["diff", "--cached", "-U0"], root)
    tokens = {"UFUNCTION":0, "UPROPERTY":0, "CLASS":0, "ENUM":0, "STRUCT":0}
    for line in out.splitlines():
        if line.startswith('+') and not line.startswith('+++'):
            if 'UFUNCTION' in line: tokens["UFUNCTION"] += 1
            if 'UPROPERTY' in line: tokens["UPROPERTY"] += 1
            if re.search(r"\bclass\b", line, re.I): tokens["CLASS"] += 1
            if re.search(r"\benum\b", line, re.I): tokens["ENUM"] += 1
            if re.search(r"\bstruct\b", line, re.I): tokens["STRUCT"] += 1
    # top dirs (group by directory, not filename). Use up to top-2 levels.
    names = run_git(["diff","--cached","--name-only"], root).splitlines()
    dir_counts = {}
    for p in names:
        if not p: continue
        d = os.path.dirname(p).replace('\\','/')
        parts = [seg for seg in d.split('/') if seg]
        seg = '/'.join(parts[:min(len(parts), 2)]) if parts else '.'
        dir_counts[seg] = dir_counts.get(seg,0) + 1
    top_dirs = [k for k,_ in sorted(dir_counts.items(), key=lambda x:x[1], reverse=True)[:3]]
    # ext counts by total delta
    num = staged_numstat(root)
    ext_counts = {}
    for p,st in num.items():
        _, ext = os.path.splitext(p)
        ext = ext.lower()
        ext_counts[ext] = ext_counts.get(ext,0) + st['adds'] + st['dels']
    top_ext = [f"{k}:{v}" for k,v in sorted(ext_counts.items(), key=lambda x:x[1], reverse=True)[:3] if k]
    return tokens, top_dirs, top_ext

def main():
    if os.environ.get('SKIP_AUTO_COMMIT_MSG'):
        return 0
    start = os.getcwd()
    root = repo_root(start)
    msg_path = None
    # default to .git/COMMIT_EDITMSG if not specified
    for i,a in enumerate(sys.argv[1:]):
        pass
    # simple args parse
    args = sys.argv[1:]
    allow_overwrite = False
    to_clip = False
    i = 0
    while i < len(args):
        a = args[i]
        if a.lower() == '-msgpath' and i+1 < len(args):
            msg_path = args[i+1]; i += 2; continue
        if a.lower() == '-allowoverwrite':
            allow_overwrite = True; i += 1; continue
        if a.lower() == '-toclipboard':
            to_clip = True; i += 1; continue
        i += 1
    if not msg_path:
        msg_path = os.path.join(root, '.git', 'COMMIT_EDITMSG')

    files = staged_name_status(root)
    num = staged_numstat(root)
    adds = sum(st.get('adds',0) for st in num.values()) if num else 0
    dels = sum(st.get('dels',0) for st in num.values()) if num else 0
    count = len(files)
    # If no staged files, still write placeholder files so callers don't fail
    if not files:
        subject = "chore: 파일 0개 변경 (+0/-0)"
        lines = [subject, "", "# 변경 없음 (staged 없음)"]
        content = "\r\n".join(lines) + "\r\n"
        with open(msg_path, 'w', encoding='utf-8-sig', newline='\r\n') as f:
            f.write(content)
        out_txt = os.path.join(os.path.dirname(__file__), 'commit_message.txt')
        with open(out_txt, 'w', encoding='utf-8-sig', newline='\r\n') as f:
            f.write(content)
        return 0
    # else proceed normally

    branch = run_git(["rev-parse","--abbrev-ref","HEAD"], root).strip()
    ctype = guess_type(files, branch)
    scopes = [guess_scope(f["path"]) for f in files]
    # most common scope
    scope = None
    if scopes:
        freq = {}
        for s in scopes: freq[s] = freq.get(s,0)+1
        scope = max(freq.items(), key=lambda x:x[1])[0]
    subject = build_subject(ctype, scope if scope!='core' else None, adds, dels, count)
    # Normalize Korean spacing like "개변경" -> "개 변경"
    try:
        subject = subject.replace("개변경", "개 변경")
    except Exception:
        pass
    # Normalize spacing like "개변경" -> "개 변경"
    try:
        subject = subject.replace("개변경", "개 변경")
    except Exception:
        pass

    tokens, top_dirs, top_ext = analyze_changes(root)

    lines = [subject, "", "# 요약", "# - 커밋 전 이 요약을 다듬어 주세요.", "", "# 주요 변경 요약",
             f"# - 총 변경량: +{adds}/-{dels}, 파일: {count}"]
    if top_dirs:
        lines.append("# - 영향 경로 상위: " + ', '.join(top_dirs))
    if top_ext:
        lines.append("# - 변경 확장자 상위: " + ', '.join(top_ext))
    if any(tokens.values()):
        lines.append("# - 코드 토큰: " + ", ".join([f"{k}:{v}" for k,v in tokens.items()]))
    lines.append("")
    lines.append("# 변경 내역(상위 10개)")
    # top 10 by adds+dels
    ranked = sorted(({ 'path':p, 'adds':st.get('adds',0), 'dels':st.get('dels',0) } for p,st in num.items()), key=lambda x:x['adds']+x['dels'], reverse=True)[:10]
    for f in ranked:
        # status lookup
        st = next((x['status'] for x in files if x['path']==f['path']), 'M')
        lines.append(f"# - {st} {f['path']} (+{f['adds']}/-{f['dels']})")
    if branch:
        lines += ["", f"# 브랜치: {branch}"]

    # Recompose with refined Korean headings and spacing
    try:
        ranked = sorted(({ 'path':p, 'adds':st.get('adds',0), 'dels':st.get('dels',0) } for p,st in num.items()), key=lambda x:x['adds']+x['dels'], reverse=True)[:10]
    except Exception:
        ranked = []
    lines = [
        subject,
        "",
        "# 요약",
        "# - 커밋 전, 아래 요약을 간단히 다듬어 주세요.",
        "",
        "# 주요 변경 요약",
        f"# - 총 변경량: +{adds}/-{dels}, 파일: {count}",
    ]
    if top_dirs:
        lines.append("# - 영향 경로 상위: " + ', '.join(top_dirs))
    if top_ext:
        lines.append("# - 변경 확장자 상위: " + ', '.join(top_ext))
    if any(tokens.values()):
        lines.append("# - 코드 토큰: " + ", ".join([f"{k}:{v}" for k,v in tokens.items()]))
    lines.append("")
    lines.append("# 변경 파일(상위 10)")
    for f in ranked:
        st = next((x['status'] for x in files if x['path']==f['path']), 'M')
        lines.append(f"# - {st} {f['path']} (+{f['adds']}/-{f['dels']})")
    if branch:
        lines += ["", f"# 브랜치: {branch}"]

    content = "\r\n".join(lines) + "\r\n"
    # Rebuild content with refined Korean summary and headings
    try:
        ranked = sorted(({'path': p, 'adds': st.get('adds', 0), 'dels': st.get('dels', 0)} for p, st in num.items()), key=lambda x: x['adds'] + x['dels'], reverse=True)[:10]
        top_files = [f['path'] for f in ranked[:3]]

        # Build a concise natural summary line from ext mix and stats
        total_delta = sum((st.get('adds', 0) + st.get('dels', 0)) for st in num.values()) or 1
        ext_mix = {}
        for p, st in num.items():
            _, e = os.path.splitext(p)
            e = e.lower()
            ext_mix[e] = ext_mix.get(e, 0) + st.get('adds', 0) + st.get('dels', 0)
        dominant = sorted(ext_mix.items(), key=lambda x: x[1], reverse=True)
        phrases = []
        if dominant:
            top_ext, top_val = dominant[0]
            share = top_val / total_delta
            if top_ext == '.md' and share >= 0.5:
                phrases.append('문서 대량 업데이트')
            elif top_ext in ('.cpp', '.h', '.hpp', '.c') and share >= 0.5:
                phrases.append('C++ 코드 중심 변경')
            elif top_ext == '.py' and share >= 0.5:
                phrases.append('도구 스크립트 중심 변경')
        if adds and not dels:
            phrases.append('추가 위주')
        elif dels and not adds:
            phrases.append('삭제 위주')
        if any(tokens.values()):
            phrases.append('UE 매크로/타입 변경 포함')
        if not phrases:
            phrases.append('변경 사항 업데이트')
        one_liner = ', '.join(dict.fromkeys(phrases))

        lines = [
            subject,
            "",
            "# 요약",
            f"# - 요약: {one_liner}",
            f"# - 총 변경량: +{adds}/-{dels}, 파일: {count}",
        ]
        if top_files:
            lines.append("# - 주요 파일: " + ', '.join(top_files))
        if top_dirs:
            lines.append("# - 영향 경로 상위: " + ', '.join(top_dirs))
        if top_ext:
            lines.append("# - 변경 확장자 상위: " + ', '.join(top_ext))
        if any(tokens.values()):
            lines.append("# - 코드 토큰: " + ", ".join([f"{k}:{v}" for k, v in tokens.items()]))
        lines.append("")
        lines.append("# 변경 내역 상위 10개")
        for f in ranked:
            st = next((x['status'] for x in files if x['path'] == f['path']), 'M')
            lines.append(f"# - {st} {f['path']} (+{f['adds']}/-{f['dels']})")
        if branch:
            lines += ["", f"# 브랜치: {branch}"]
        content = "\r\n".join(lines) + "\r\n"
    except Exception:
        pass
    # write UTF-8 with BOM for Windows editors
    with open(msg_path, 'w', encoding='utf-8-sig', newline='\r\n') as f:
        f.write(content)
    # also write a copy under Tools
    out_txt = os.path.join(os.path.dirname(__file__), 'commit_message.txt')
    with open(out_txt, 'w', encoding='utf-8-sig', newline='\r\n') as f:
        f.write(content)

    # optional clipboard
    if to_clip:
        try:
            import subprocess
            p = subprocess.Popen(['clip'], stdin=subprocess.PIPE)
            p.communicate(input=content.encode('utf-8'))
        except Exception:
            pass
    return 0

if __name__ == '__main__':
    sys.exit(main())
