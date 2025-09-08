Repository Guidelines (DragonBallSZ)

이 파일은 리포지토리 전역(루트) 기준으로 적용됩니다. 에이전트/기여자가 프로젝트를 탐색·수정할 때 따라야 할 규칙과 참고 정보를 담고 있습니다.

Project Structure & Modules
- `Source/DragonBallSZ`: 게임 코드(C++). `Public/`에는 헤더, `Private/`에는 소스.
- `Source/CoffeeLibrary`: 공용 유틸리티 모듈. 동일하게 `Public/`/`Private/` 구조.
- `Content/`: 에셋(메시, 머티리얼, 블루프린트 등).
- `Config/`: 엔진/프로젝트 설정.
- `Tests/` (옵션): Automation/Functional 테스트.

모듈 구성
- 게임 모듈: `DragonBallSZ`
- 라이브러리 모듈: `CoffeeLibrary`

Build & Run
- IDE 빌드(권장): Rider 또는 Visual Studio에서 `DragonBallSZ.uproject`를 열고 `Build → Rebuild`.
- 명령행 빌드(Windows)
  - 에디터 타겟: `"<UE>\\Engine\\Build\\BatchFiles\\Build.bat" DragonBallSZEditor Win64 Development -Project="<PATH>\\DragonBallSZ.uproject" -WaitMutex`
  - 게임 타겟: `"<UE>\\Engine\\Build\\BatchFiles\\Build.bat" DragonBallSZ Win64 Development -Project="<PATH>\\DragonBallSZ.uproject" -WaitMutex`
  - (옵션) 프로젝트 파일 생성: `"<UE>\\Engine\\Build\\BatchFiles\\GenerateProjectFiles.bat" -project="<PATH>\\DragonBallSZ.uproject"`
- 실행: 언리얼 에디터에서 레벨 열고 Play.
- 핫리로드: C++ 변경 후 에디터 `Compile` 버튼 또는 IDE에서 빌드.

Coding Style & Naming
- UE 스타일 준수: `UCLASS`/`USTRUCT`/`UFUNCTION` 매크로, 타입/클래스 PascalCase, 변수 camelCase.
- `UPROPERTY`/`UPARAM` 메타 유지(가시성, 에디터 노출, 직렬화 등 의도 반영).
- 폴더 네임스페이스 예시
  - 게임: `Source/DragonBallSZ/<Domain>/...` (`Character`, `Enviroment`, `Common` 등)
  - 라이브러리: `Source/CoffeeLibrary/<Domain>/...`
- 로그 카테고리: `DECLARE_LOG_CATEGORY_EXTERN` / `DEFINE_LOG_CATEGORY`로 모듈별 정의.

Testing
- Automation 기반 단위 테스트(`AutomationTest.h`) 권장.
- 파일명: `*Tests.cpp`
- 테스트명: `Project.Feature.Scenario`
- 실행: 에디터 `Session Frontend → Automation`.

Git & PR
- 브랜치 접두사: `feat/`, `fix/`, `chore/`.
- 커밋 메시지: `type(scope): summary` (예: `fix(skill): null check in AddSkill`).
- PR 요구사항: 변경 요약, 스크린샷/로그(필요 시), 관련 이슈 링크, 테스트 결과.

Notes
- `.uproject`: `DragonBallSZ.uproject`
- 주요 코드 영역: `Character`, `Enviroment`, `Common`, `CoffeeLibrary`(`Actor`/`Features`/`Shared`/`Core`).
- 대용량 에셋(`Content/`)과 `Saved/` 산출물은 버전 관리에서 제외 권장.
