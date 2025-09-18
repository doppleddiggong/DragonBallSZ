# Debugging Progress for URushAttackSystem and AEnemyActor

## Date: 2025년 9월 16일

## 1. Identified Issues & Root Causes

### 1.1. Crash in `URushAttackSystem::DashToTarget` (Division by Zero)
- **Symptom:** Crash occurring in `URushAttackSystem::DashToTarget` around line 307.
- **Root Cause:** Division by zero when `Owner` and `Target` actors were at the exact same XY location, leading to `DistanceXY` being zero. This resulted in `NaN` or `Inf` values in `DirXY`, which could cause subsequent engine calculations to fail.

### 1.2. Assertion Failure: `Index >= 0` in `UObjectArray.h`
- **Symptom:** Low-level engine assertion failure (`Index >= 0`) when running the UnrealEditor executable.
- **Root Causes Identified & Addressed:**
    - **Invalid `Target` Actor:** `URushAttackSystem` was attempting to use `Target` (derived from `Owner->TargetActor`) without validating its existence. If `Target` was `nullptr`, dereferencing it would lead to a crash or corruption.
    - **Invalid `Owner` Actor during Dash:** While dashing (`bIsDashing` is true), `URushAttackSystem::TickComponent` was directly using `Owner` without validation. If `Owner` became invalid (e.g., destroyed), accessing its properties would crash.
    - **Missing `UCharacterData` Asset (`VEGE_DATA`):** In `AEnemyActor::BeginPlay()`, the `CharacterData` (loaded from `VEGE_DATA`) was used without checking if it was successfully loaded. If `VEGE_DATA` was missing or corrupted, `CharacterData` would be `nullptr`, leading to dereference crashes.
    - **`ComboCount` out of bounds for `AttackPowerType`:** Although `ComboCount` was managed for `AttackMontages`, `AttackPowerType` was accessed with `ComboCount` without an `IsValidIndex` check, assuming parallel array sizes.
    - **`AnimInstance` becoming null:** `AnimInstance` was used for `Montage_Play`/`Montage_Stop` calls without `IsValid` checks. If `AnimInstance` became null (e.g., character mesh destroyed), these calls would crash.

## 2. Applied Fixes

The following changes have been applied to enhance robustness and prevent the identified crashes:

### 2.1. `URushAttackSystem.cpp`
-   **`DashToTarget(int32 MontageIndex)`**:
    -   Added `if (DistanceXY < KINDA_SMALL_NUMBER)` check to prevent division by zero. If true, `PlayMontage(MontageIndex)` is called, and the function returns.
    -   Added `if (!IsValid(Target))` check at the beginning. If `Target` is invalid, `PlayMontage(MontageIndex)` is called, and the function returns.
    -   Added `if (IsValid(AnimInstance))` check before `AnimInstance->Montage_Play(DashMontage, ...)`.
-   **`InitSystem(ACombatCharacter* InOwner, UCharacterData* InData)`**:
    -   Added `if (IsValid(Target))` check before `TargetMoveComp = Target->GetCharacterMovement();`.
-   **`OnAttack()`**:
    -   Added `if (!IsValid(Target))` check at the beginning. If `Target` is invalid, the function returns.
-   **`TickComponent(...)`**:
    -   Added `if (!IsValid(Owner))` check within the `if (bIsDashing)` block. If `Owner` is invalid, `OnDashCompleted()` is called, and the function returns.
-   **`AttackTrace()`**:
    -   Added `if (!AttackPowerType.IsValidIndex(ComboCount))` check before accessing `AttackPowerType[ComboCount]`. If out of bounds, logs an error and returns.
-   **`OnDashCompleted()`**:
    -   Added `if (IsValid(AnimInstance))` check before `AnimInstance->Montage_Stop(...)`.
-   **`PlayMontage(int32 MontageIndex)`**:
    -   Added `if (IsValid(AnimInstance))` check before `AnimInstance->Montage_Play(...)`.

### 2.2. `AEnemyActor.cpp`
-   **`BeginPlay()`**:
    -   Added `if (!IsValid(CharacterData))` check at the beginning. If `CharacterData` is invalid, logs an error and returns.

## 3. Next Steps & Considerations

-   **Compile and Test:** The project should be recompiled, and thorough testing should be performed to verify that the identified crashes are resolved and no new issues have been introduced.
-   **Asset Validation:** Ensure that the `VEGE_DATA` asset (`/Game/CustomContents/MasterData/Vege_Data.Vege_Data`) exists and is valid. Missing or corrupted assets can still cause issues, even with the added null checks.
-   **Further `IsValid` Checks:** While critical points have been covered, a comprehensive review of all raw pointer usage in `URushAttackSystem` and `AEnemyActor` for `IsValid` checks (especially for `Owner`, `Target`, `AnimInstance`, `MeshComp`, `MoveComp`, `EventManager`, `UDBSZDataManager`) is recommended for maximum robustness.
-   **`TargetActor` Lifecycle:** Investigate how `Owner->TargetActor` is set and if it can become invalid during gameplay without being explicitly cleared or updated.
-   **`ComboCount` and Parallel Arrays:** Reconfirm that `UCharacterData::LoadRushAttackMontage` consistently populates `AttackMontages` and `AttackPowerType` with matching sizes.

This document will serve as a reference for future debugging sessions and development efforts related to the `URushAttackSystem` and `AEnemyActor` components.