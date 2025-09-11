#!/usr/bin/env python3
import os, sys, json, datetime

def kst_now():
    return datetime.datetime.utcnow() + datetime.timedelta(hours=9)

def parse_args(argv):
    # -Question "..." -Answer "..." -User "..." -Tags "a;b"
    args = { 'Question': None, 'Answer': None, 'User': os.environ.get('USERNAME','user'), 'Tags': '' }
    i=0
    while i < len(argv):
        a = argv[i]
        if a.lower() == '-question' and i+1 < len(argv): args['Question']=argv[i+1]; i+=2; continue
        if a.lower() == '-answer' and i+1 < len(argv): args['Answer']=argv[i+1]; i+=2; continue
        if a.lower() == '-user' and i+1 < len(argv): args['User']=argv[i+1]; i+=2; continue
        if a.lower() == '-tags' and i+1 < len(argv): args['Tags']=argv[i+1]; i+=2; continue
        i+=1
    return args

def main():
    start = os.getcwd()
    # repo root
    root = start
    # find .git up the tree
    cur = start
    while True:
        if os.path.isdir(os.path.join(cur,'.git')):
            root = cur; break
        parent = os.path.dirname(cur)
        if parent == cur: break
        cur = parent

    args = parse_args(sys.argv[1:])
    if not args['Question'] or not args['Answer']:
        print('[AgentQA] Missing -Question or -Answer')
        return 1
    # Spec: Documents/AgentQA (versioned)
    doc_root = os.path.join(root, 'Documents', 'AgentQA')
    os.makedirs(doc_root, exist_ok=True)
    now = kst_now()
    date = now.strftime('%Y-%m-%d')
    time = now.strftime('%H%M%S')
    md_path = os.path.join(doc_root, f'{date}.md')
    jsonl = os.path.join(doc_root, 'qa_log.jsonl')
    tags = [t.strip() for t in re.split(r'[;,]', args['Tags'])] if args['Tags'] else []
    entry = {
        'ts': now.isoformat(),
        'user': args['User'],
        'tags': tags,
        'question': args['Question'],
        'answer': args['Answer']
    }
    # write md
    with open(md_path, 'a', encoding='utf-8') as f:
        if os.path.getsize(md_path) == 0:
            f.write(f"# Agent Q&A - {date} (KST)\n\n")
        f.write(f"## {time} — {args['User']}\n")
        if tags:
            f.write(f"Tags: {', '.join(tags)}\n")
        f.write("### Q\n" + args['Question'] + "\n")
        f.write("### A\n" + args['Answer'] + "\n\n")
    # write jsonl
    with open(jsonl, 'a', encoding='utf-8') as f:
        f.write(json.dumps(entry, ensure_ascii=False) + "\n")
    print(f"[AgentQA] Saved to: {md_path}")
    print(f"[AgentQA] JSONL appended: {jsonl}")
    return 0

if __name__ == '__main__':
    import re
    sys.exit(main())

