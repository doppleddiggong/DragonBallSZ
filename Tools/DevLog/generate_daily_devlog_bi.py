#!/usr/bin/env python3
import os, sys, subprocess, re, json, datetime

# Simple fixed KST (+09:00) tzinfo
class KST(datetime.tzinfo):
    def utcoffset(self, dt): return datetime.timedelta(hours=9)
    def tzname(self, dt): return 'KST'
    def dst(self, dt): return datetime.timedelta(0)

def run_git(args, cwd):
    try:
        cp = subprocess.run(["git"] + args, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, encoding='utf-8', errors='ignore')
        if cp.returncode != 0:
            return ""
        return cp.stdout
    except Exception:
        return ""

def project_root(start_dir):
    out = run_git(["rev-parse","--show-toplevel"], start_dir).strip()
    return out or start_dir

def get_kst_now():
    return datetime.datetime.utcnow().replace(tzinfo=datetime.timezone.utc).astimezone(KST())

def day_bounds_kst(now_kst):
    d = now_kst.date()
    today9 = datetime.datetime(d.year, d.month, d.day, 9, 0, 0, tzinfo=KST())
    if now_kst < today9:
        d = d - datetime.timedelta(days=1)
        today9 = datetime.datetime(d.year, d.month, d.day, 9, 0, 0, tzinfo=KST())
    yest9 = today9 - datetime.timedelta(days=1)
    return d.strftime('%Y-%m-%d'), yest9.isoformat(), today9.isoformat()

def bounds_for_date_kst(d):
    # d is date (naive)
    end = datetime.datetime(d.year, d.month, d.day, 9, 0, 0, tzinfo=KST())
    start = end - datetime.timedelta(days=1)
    return d.strftime('%Y-%m-%d'), start.isoformat(), end.isoformat()

def git_log_commits(root, since_iso, until_iso, no_merges=True):
    args = ["log"]
    if no_merges: args.append("--no-merges")
    args += [f"--since={since_iso}", f"--until={until_iso}", "--date=iso-strict",
             "--pretty=format:%H|%ad|%an|%s%n%b", "--numstat"]
    out = run_git(args, root)
    commits = []
    cur = None
    for line in out.splitlines():
        m = re.match(r"^(?P<sha>[0-9a-f]{7,40})\|(?P<date>[^|]+)\|(?P<author>[^|]+)\|(?P<sub>.*)$", line)
        if m:
            if cur: commits.append(cur)
            cur = {"sha":m.group('sha'),"date":m.group('date'),"author":m.group('author'),
                   "subject":m.group('sub'),"body":[],"adds":0,"dels":0,"files":0}
            continue
        m2 = re.match(r"^(?P<adds>\d+|-)\t(?P<dels>\d+|-)\t(?P<path>.+)$", line)
        if m2 and cur:
            adds = 0 if m2.group('adds')=='-' else int(m2.group('adds'))
            dels = 0 if m2.group('dels')=='-' else int(m2.group('dels'))
            cur["adds"] += adds; cur["dels"] += dels; cur["files"] += 1
            continue
        if cur is not None:
            cur["body"].append(line)
    if cur: commits.append(cur)
    # extract simple TODO/breaking flags
    for c in commits:
        body_text = "\n".join(c["body"]) if c["body"] else ""
        todos = re.findall(r"(?im)^\s*(?:todo:|@todo)\s*(.+)$", body_text)
        c["todos"] = [t.strip() for t in todos]
        c["breaking"] = bool(re.search(r"(?i)breaking change", body_text))
    return commits

def classify(subject):
    m = re.match(r"^(?P<type>[a-zA-Z]+)(\((?P<scope>[^)]+)\))?:\s*(?P<sub>.+)$", subject or "")
    ctype = (m.group('type') if m else '').lower()
    scope = (m.group('scope') if m else None)
    sub = (m.group('sub') if m else (subject or ''))
    cl = 'Progress'
    if ctype:
        if re.match(r"^(feat|perf|refactor|style|build|revert|docs|test|ci|fix)$", ctype): cl = 'Done'
        elif re.match(r"^(chore|wip)$", ctype): cl = 'Progress'
    else:
        if re.search(r"\bfix|bug|hotfix\b", sub, re.I): cl = 'Done'
        elif re.search(r"\bwip|partial|temp\b", sub, re.I): cl = 'Progress'
    return cl, scope, sub

def render_day_md(root, date_str, commits, out_path, template_tools, template_doc):
    addsT = sum(c['adds'] for c in commits)
    delsT = sum(c['dels'] for c in commits)
    filesT = sum(c['files'] for c in commits)
    done=[]; prog=[]; need=[]; todos_list=[]; details_lines=[]
    for c in commits:
        cl, scope, clean_sub = classify(c['subject'])
        sha7 = c['sha'][:7]
        scope_tag = f"[{scope}] " if scope else ""
        line = f"{scope_tag}{clean_sub} ({sha7}) (+{c['adds']}/-{c['dels']}, {c['files']} files)"
        if cl=='Done': done.append(line)
        else: prog.append(line)
        if c['breaking']:
            need.append(f"주의: 호환성 변경/마이그레이션 필요 ({sha7})")
        for t in c.get('todos',[]):
            todos_list.append(f"- [ ] {t} (출처: {sha7})")
        # Commit body details (quoted lines)
        body_text = "\n".join(c.get("body", [])).strip()
        if body_text:
            details_lines.append(f"- {scope_tag}{clean_sub} ({sha7}) — {c['author']} @ {c['date']}")
            for bl in body_text.splitlines():
                details_lines.append(f"  > {bl}")
            details_lines.append("")

    title_en = f"# Daily DevLog {date_str} (KST 09:00 boundary)"
    title_ko = f"# 일일 개발 로그 {date_str} (KST 09:00 경계)"
    summary_en = f"Commits: {len(commits)} | Changes: +{addsT} / -{delsT} | Files: {filesT}"
    summary_ko = f"커밋: {len(commits)}개 | 변경 합계: +{addsT} / -{delsT} | 파일: {filesT}"
    high_en = "2-5 high-impact changes, short bullets"
    high_ko = "영향 큰 변경 2~5개를 간단히 요약"
    done_head_en, done_head_ko = '### Done','### 완료'
    prog_head_en, prog_head_ko = '### In Progress','### 진행'
    need_head_en, need_head_ko = '### Needs Attention','### 주의 필요'
    todo_head_en, todo_head_ko = '### TODO (from commits)','### 커밋 기반 TODO'

    done_body = "(none) / (없음)" if not done else "\r\n".join(done)
    prog_body = "(none) / (없음)" if not prog else "\r\n".join(prog)
    need_body = "(none) / (없음)" if not need else "\r\n".join(need)
    todo_body = "(none) / (없음)" if not todos_list else "\r\n".join(todos_list)

    details_head_en, details_head_ko = '## Commit Details','## 커밋 상세'
    details_body = "(none) / (?�음)" if not details_lines else "\r\n".join(details_lines)

    metrics_en = "\r\n".join([
        f"Commits: {len(commits)}",
        f"Adds: +{addsT} / Dels: -{delsT}",
        f"Files: {filesT}",
        f"Done: {len(done)} | Progress: {len(prog)} | Needs: {len(need)}",
        f"Todos: {len(todos_list)}",
    ])
    metrics_ko = "\r\n".join([
        f"커밋: {len(commits)}개",
        f"추가: +{addsT} / 삭제: -{delsT}",
        f"파일: {filesT}",
        f"완료: {len(done)} | 진행: {len(prog)} | 주의: {len(need)}",
        f"TODO: {len(todos_list)}",
    ])

    # Template
    tpl_path = None
    if template_tools and os.path.isfile(template_tools): tpl_path = template_tools
    elif template_doc and os.path.isfile(template_doc): tpl_path = template_doc

    if tpl_path:
        with open(tpl_path, 'r', encoding='utf-8') as f:
            tpl = f.read()
        tokens = {
            'DATE': date_str,
            'TITLE_EN': title_en,
            'TITLE_KO': title_ko,
            'SUMMARY_EN': summary_en,
            'SUMMARY_KO': summary_ko,
            'HIGHLIGHTS_EN': high_en,
            'HIGHLIGHTS_KO': high_ko,
            'DONE_EN_HEAD': done_head_en,
            'DONE_KO_HEAD': done_head_ko,
            'DONE_BODY': done_body,
            'PROGRESS_EN_HEAD': prog_head_en,
            'PROGRESS_KO_HEAD': prog_head_ko,
            'PROGRESS_BODY': prog_body,
            'NEEDS_EN_HEAD': need_head_en,
            'NEEDS_KO_HEAD': need_head_ko,
            'NEEDS_BODY': need_body,
            'TODO_EN_HEAD': todo_head_en,
            'TODO_KO_HEAD': todo_head_ko,
            'TODO_BODY': todo_body,
            'METRICS_EN_HEAD': '## Metrics (approx)',
            'METRICS_KO_HEAD': '## 메트릭(추정)',
            'METRICS_EN': metrics_en,
            'METRICS_KO': metrics_ko,
        }
        md = tpl
        for k,v in tokens.items():
            md = md.replace("{{"+k+"}}", v)
    else:
        parts = [
            title_en, title_ko, "", "## Summary / 요약", summary_en, summary_ko, "",
            "## Highlights / 하이라이트", high_en, high_ko, "",
            "## Commit-based Work Log", "## 커밋 기반 작업 로그", "",
            done_head_en, done_head_ko, done_body, "",
            prog_head_en, prog_head_ko, prog_body, "",
            need_head_en, need_head_ko, need_body, "",
            todo_head_en, todo_head_ko, todo_body, "",
            details_head_en, details_head_ko, details_body, "",
            "## Metrics (approx)", "## 메트릭(추정)", metrics_en, metrics_ko, ""
        ]
        md = "\r\n".join(parts)

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'w', encoding='utf-8') as f:
        f.write(md)

    # metrics JSON
    metrics = {
        'date': date_str, 'commits': len(commits), 'adds': addsT, 'dels': delsT,
        'files': filesT, 'done': len(done), 'progress': len(prog), 'needsAttention': len(need), 'todos': len(todos_list)
    }
    with open(os.path.join(os.path.dirname(out_path), f"{date_str}.metrics.json"), 'w', encoding='utf-8') as f:
        json.dump(metrics, f, ensure_ascii=False, indent=2)

def build_last30_summary(root, out_dir):
    now = get_kst_now().date()
    texts=[]
    for i in range(30):
        d = now - datetime.timedelta(days=i)
        p = os.path.join(out_dir, d.strftime('%Y-%m-%d') + '.md')
        if os.path.isfile(p):
            with open(p,'r',encoding='utf-8', errors='ignore') as f:
                texts.append(f.read())
    if not texts: return
    done=prog=need=todo=0
    import re
    for t in texts:
        done += len(re.findall(r"(?m)^### Done", t))
        prog += len(re.findall(r"(?m)^### In Progress", t))
        need += len(re.findall(r"(?m)^### Needs Attention", t))
        todo += len(re.findall(r"(?m)^- \[ \]", t))
    lines=[f"## 30-Day Briefing {now.strftime('%Y-%m-%d')}", "",
           "### Overview / 개요",
           f"- Daily files: {len(texts)} (last 30 days)",
           f"- 일자 파일 수: {len(texts)} (최근 30일)",
           f"- TODO outstanding (est.): {todo}",
           f"- TODO 미완료(추정): {todo}",
           "",
           "### Suggested Focus / 권장 가이드(요약)",
           "1) Prioritize open TODOs and confirm schedule",
           "1) 미해결 TODO 우선 처리 및 일정 확인",
           "2) Review commit days with many in-progress items",
           "2) 진행 항목이 많은 날짜의 커밋 검토(분할/종결 여부 확인)",
           "3) Improve Done/Progress ratio (smaller commits)",
           "3) 완료 대비 진행 비율 개선(작은 단위 커밋 권장)",
           ""]
    os.makedirs(out_dir, exist_ok=True)
    with open(os.path.join(out_dir,'_Last30Summary.md'),'w',encoding='utf-8') as f:
        f.write("\n".join(lines))

def main():
    # parse simple args: -BackfillDays N -BuildSummary -NoMerges
    args = sys.argv[1:]
    backfill = 0
    build_summary = True
    no_merges = True
    i=0
    while i < len(args):
        a = args[i].lower()
        if a == '-backfilldays' and i+1 < len(args):
            try: backfill = int(args[i+1])
            except: backfill = 0
            i += 2; continue
        if a == '-buildsummary':
            build_summary = True; i += 1; continue
        if a == '-nomerges':
            no_merges = True; i += 1; continue
        i += 1

    start = os.getcwd()
    root = project_root(start)
    out_dir = os.path.join(root, 'Documents', 'DevLog')
    tools_tpl = os.path.join(root, 'Tools', 'template.md')
    doc_tpl = os.path.join(out_dir, 'template.md')

    # today (if missing)
    today_str, start_iso, end_iso = day_bounds_kst(get_kst_now())
    today_path = os.path.join(out_dir, today_str + '.md')
    if not os.path.isfile(today_path):
        commits = git_log_commits(root, start_iso, end_iso, no_merges)
        render_day_md(root, today_str, commits, today_path, tools_tpl, doc_tpl)

    # backfill
    if backfill > 0:
        now = get_kst_now().date()
        for i in range(1, backfill+1):
            d = now - datetime.timedelta(days=i)
            date_str, s_iso, e_iso = bounds_for_date_kst(d)
            out_path = os.path.join(out_dir, date_str + '.md')
            if os.path.isfile(out_path):
                continue
            commits = git_log_commits(root, s_iso, e_iso, no_merges)
            render_day_md(root, date_str, commits, out_path, tools_tpl, doc_tpl)

    if build_summary:
        build_last30_summary(root, out_dir)
    return 0

if __name__ == '__main__':
    sys.exit(main())

