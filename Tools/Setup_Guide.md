# Tools Setup Guide

이 문서는 Tools 폴더 스크립트를 기능별로 정리하고, Fork에서 Bash Command로 실행하는 방법을 안내합니다.

## 기능 개요
- Commit 메시지 자동 생성: Conventional Commit 제목/요약 생성 + 클립보드 복사
- DevLog 생성: 커밋 기반 일일 개발 로그 생성/갱신
- 프로젝트 초기화: Python 확인/설치, Git 훅 설치, DevLog 1회 실행
- 기타: Q&A 로그 저장

## 폴더 구조(기능별)
- Commit 메시지
  - `Tools/CommitMessage/gen_commit_msg.py` — 생성 로직
  - `Tools/CommitMessage/run_gen_commit_msg.sh` — Bash 래퍼
  - `Tools/RunGenCommitMsgPy.cmd` — Windows 래퍼(클립보드 자동 복사)
  - 출력: `Tools/commit_message.txt`, `.git/COMMIT_EDITMSG`
- DevLog
  - `Tools/DevLog/run_devlog.sh` — Bash 래퍼
  - `Tools/RunDevLogPy.cmd` — Windows 래퍼
  - `Tools/run_generate_daily_devlog_once.py`, `Tools/generate_daily_devlog_bi.py`, `Tools/template.md`
- 초기화/설치
  - `Tools/SetupProject.cmd`, `Tools/Install/python-*.exe`
- 기타
  - `Tools/SaveAgentQA/save_agent_qa.py`, `Tools/ps1/`, `Tools/Utils/`

## Fork 설정(권장)
Custom Actions → Bash Command에 아래 항목을 추가하세요.

1) Generate Commit Message
- Command: `bash -lc "cd '{RepositoryRoot}' && ./Tools/CommitMessage/run_gen_commit_msg.sh"`
- Working directory: `{RepositoryRoot}`

2) Generate DevLog
- Command: `bash -lc "cd '{RepositoryRoot}' && ./Tools/DevLog/run_devlog.sh"`
- Working directory: `{RepositoryRoot}`

참고) PowerShell로 직접 실행
- Commit: `powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "& '{RepositoryRoot}\Tools\RunGenCommitMsgPy.cmd'"`
- DevLog: `powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "& '{RepositoryRoot}\Tools\RunDevLogPy.cmd'"`

## 개별 사용법
### Commit 메시지
- Bash: `./Tools/CommitMessage/run_gen_commit_msg.sh`
- Windows: `Tools\RunGenCommitMsgPy.cmd`
- Git 훅(선택): `py -3 Tools\CommitMessage\install_commit_hook.py`

### DevLog
- Bash: `./Tools/DevLog/run_devlog.sh`
- Windows: `Tools\RunDevLogPy.cmd`

### 프로젝트 초기화
- `Tools\SetupProject.cmd` — Python 확인 → 훅 설치 → DevLog 1회 실행

### Agent Q&A 저장
- 예: `py -3 Tools\SaveAgentQA\save_agent_qa.py -Question "..." -Answer "..." -User user -Tags "tag1,tag2"`

## 비고
- 비대화형 실행이 필요하면 래퍼의 pause 제거/토글 추가 가능(요청 시 적용).
- 콘솔에서 일부 문자가 깨져 보여도 파일은 UTF-8(BOM)으로 저장됩니다.

