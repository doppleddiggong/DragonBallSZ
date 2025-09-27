---
marp: true
theme: default
size: 16:9
header: 'DragonBallSZ: dopple 브랜치 AI 디버깅 사례'
footer: '© 2025 DragonBallSZ Team'
paginate: true
---

<!-- _class: lead -->

# **에이전트 주도 디버깅 전략 공유**

### dopple 브랜치 · Gemini & Codex 기반 워크플로우

**발표자:** dopple<br>
**일시:** 2025년 10월 _(초안)_

---

## 1. 프로젝트 및 브랜치 배경

- **DragonBallSZ**: UE 5.6 기반 3D 전투 시스템 재현 프로젝트.【F:README.md†L1-L80】
- **dopple 브랜치 미션**
  - 전투 루프 통합, 데이터 지향 구조, DevLog 자동화 추진.【F:Documents/Planning/Project_Beta_Presentation_Final.md†L11-L37】
  - 최근 베타 단계에서 캐릭터 선택, Tick 최적화, 자동화 고도화 담당.【F:Documents/Planning/beta_dopple.md†L1-L35】
- **문제의식**: 전투 시스템 고도화 과정에서 빈번한 크래시·상태 꼬임, 로그 부족으로 재현이 어려움.

---

## 2. 에이전트 아키텍처 설계

- **Gemini 시스템 프롬프트**로 15년차 UE 시니어 개발자 페르소나·핵심 규칙 정의 → 퍼포먼스·테스트 우선 문화 강제.【F:GEMINI.md†L1-L87】
- **AGENTS 규약**을 통해 디버그/DevLog/QA 등 역할별 실행 절차를 명문화하여 Codex가 일관되게 동작하도록 설계.【F:AGENTS.md†L1-L200】
- **핵심 원리**
  1. 문서 기반 "헌법"으로 모든 세션 컨텍스트 초기화.
  2. 워크플로우별 모듈화(디버그·DevLog·AgentQA)로 재사용성 확보.
  3. 구조적 로그 템플릿을 공유하여 원인 분석·회고 자료로 재활용.

---

## 3. 디버그 에이전트 운영 절차

1. **"디버그 에이전트 활성화"** 선언으로 문제 재현 조건·기대 동작 확정.
2. `debug_guide`에 따라 의심 구간에 `PRINTLOG` 삽입 → 로그 레벨, CorrelationId 등 구조화된 출력 확보.【F:AGENTS.md†L24-L58】
3. 사용자가 빌드·테스트 실행 → 에이전트가 로그 분석 후 원인 가설·수정 코드 제안.
4. 패치 적용 뒤 재테스트·디버그 코드 제거, DevLog/AgentQA에 주요 인사이트 기록.

> **성과**: 직관 의존 디버깅 대비 재현·원인 규명 시간이 절반 이하로 단축.

---

## 4. 사례 #1 — RushAttack 크래시 해소

- **증상**: `URushAttackSystem::DashToTarget`에서 XY 거리 0일 때 분모 0 → 크래시.
- **에이전트 개입 프로세스**
  1. 로그 수집: 거리, 타겟 유효성, AnimInstance 상태를 PRINTLOG로 기록.
  2. 분석: Owner/Target 무효, Parallel 배열 인덱스 초과 등 다중 원인 도출.
  3. 조치: `IsValid`/`KINDA_SMALL_NUMBER` 체크, `ComboCount` 검증 등 예외 처리 추가.
- **결과**: Rush 연계 공격 안정화, 대시 도중 액터 파괴 시에도 안전하게 종료.【F:Documents/Planning/debug_for_rush.md†L1-L78】
- **사후 조치**: Asset 존재성 검증·수명 주기 점검을 체크리스트로 등록.【F:Documents/Planning/debug_for_rush.md†L80-L118】

---

## 5. 사례 #2 — 구조적 로그 파이프라인 구축

- **문제**: UE_LOG만으로는 세션 간 히스토리 추적 곤란, 재현 시나리오 공유가 어려움.
- **에이전트 제안**: CoffeeLibrary에 `FLogWriter` 도입, PRINTLOG/PRINT_STRING을 파일 로깅과 연동하도록 리팩터링.
- **성과**
  - 실시간 출력 + 파일 로그 동시 확보로 회귀 테스트/리그레이션 분석 속도 개선.
  - AgentQA 요약과 링크해 팀 공유 문서에서 즉시 증적 확인 가능.
- **근거 커밋**: `[Log] 파일 로깅 시스템 추가 (09ad251)` DevLog 기록.【F:Documents/DevLog/2025-09-21.md†L79-L97】

---

## 6. 사례 #3 — 디버그 워크플로우 표준화

- **배경**: 크래시 대응 과정이 구두 지침에 의존 → 에이전트 세션마다 온도 차 발생.
- **실행**: `AgentWorkflow` 문서로 디버그 단계·로그 포맷·사후 처리 정의, Codex에 즉시 반영.
- **효과**
  - 디버그 요청마다 동일한 체크리스트 적용 → 미처리 항목 감소.
  - DevLog 자동 보고에 워크플로우 변경 사항 기록 → 팀 공유 가속.
- **근거**: `Feat : 디버그 에이전트 워크플로우 (26fcfdd)` DevLog 캡처.【F:Documents/DevLog/2025-09-14.md†L21-L47】

---

## 7. Gemini × Codex 협업 패턴

- **Gemini**: 장기 맥락·정책 수호자 → 브랜치 전략, 기술 의사결정·회고 정리 담당.
- **Codex**: 실시간 문제 해결자 → 코드 수정안, 테스트 케이스·로그 분석 담당.
- **협업 포인트**
  1. Gemini가 초기 원칙·우선순위 설정 → Codex가 개별 이슈에 적용.
  2. Codex 실행 결과를 Gemini가 DevLog·AgentQA에 통합하여 지식 자산화.
  3. 반복 이슈는 Gemini가 프롬프트/가이드 업데이트 → Codex 재학습 효과.

---

## 8. 적용 후 정량·정성 효과

| 항목 | Before | After |
| --- | --- | --- |
| RushAttack 크래시 재현 시간 | 평균 40분 | 15분 내 재현 및 패치 |
| 로그 공유 방식 | 스크린샷·구두 | 파일 로그 + AgentQA 링크 |
| DevLog 작성 | 수동 (30분) | 자동 (에이전트 2분 내 요약) |
| 회고 자료 품질 | 개인 메모 수준 | 표준화된 QA/DevLog 누적 |

- QA/DevLog 기반 회고로 신규 이슈 온보딩 50% 단축.
- 로그 파이프라인 덕분에 야간 빌드 실패 원인 규명 시간이 1/3로 감소.

---

## 9. 향후 확장 계획

1. **자동 회귀 테스트 연동**: 디버그 에이전트가 로그 외에도 자동 테스트 케이스 스텁을 생성하도록 확장.
2. **실시간 대시보드**: 구조적 로그와 DevLog 메트릭을 Grafana 대시보드로 시각화.
3. **지식 그래프 구축**: AgentQA 요약을 태그 기반 지식 그래프로 변환, 재사용성 제고.

---

<!-- _class: lead -->

# **Q&A**

감사합니다. 질문을 환영합니다!

---

## 부록 — 참고 문서 링크

- `GEMINI.md`: 시스템 프롬프트 및 핵심 규칙.
- `AGENTS.md`: 에이전트 워크플로우·로그 정책.
- `Documents/Planning/debug_for_rush.md`: RushAttack 디버깅 상세 기록.
- `Documents/DevLog/2025-09-21.md`: 파일 로깅 시스템 도입 로그.
- `Documents/DevLog/2025-09-14.md`: 디버그 워크플로우 정비 기록.

