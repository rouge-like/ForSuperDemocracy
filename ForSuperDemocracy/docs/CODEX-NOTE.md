# CODEX NOTE

- 프로젝트 목표: TPS 게임(Helldivers 2 느낌) 프로토타입
- 요소: Unreal Engine 5 프로젝트(`ForSuperDemocracy.uproject`)

## 일정/진척
- 팀 구성: 3인 개발(가정)
- 담당(예시): 무기, UI, 시스템
- 오늘 초점: 무기 `WeaponBase`와 `WeaponComponent` 구체화

## 현재 코드/구성 요약
- 엔진/세팅: UE 5.6, 최신 IncludeOrder. `Game`/`Editor` 타깃(`Source/ForSuperDemocracy.Target.cs`, `Source/ForSuperDemocracyEditor.Target.cs`).
- 게임 모듈: `ForSuperDemocracy` 단일 모듈. 기본 모듈만 등록되어 있으며 커스텀 게임플레이는 초기 상태.
  - `Source/ForSuperDemocracy/ForSuperDemocracy.cpp`: `FDefaultGameModuleImpl`로 기본 모듈 등록/실행.
  - `Source/ForSuperDemocracy/ForSuperDemocracy.h`: 기본 헤더 포함.
  - `Source/ForSuperDemocracy/ForSuperDemocracy.Build.cs`: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput` 의존성.
- 콘텐츠: `Content/Maps/ProtoMap.umap` 존재. 현재 `DefaultEngine.ini`의 `GameDefaultMap`은 OpenWorld 템플릿으로 되어 있어 `ProtoMap`으로 교체 필요.
- 입력 설정: `EnhancedInput` 사용 예정(매핑/바인딩 정비 중).
- 렌더링: Windows DX12 + SM6, Linux Vulkan SM6. Lumen(동적 GI/리플렉션) 사용, Nanite/RT는 상황에 따라 조정.

## 현재 상태 진단
- 상태: 초기 아키텍처 단계. 캐릭터/플레이어 컨트롤러/게임모드/입력 매핑/기본 UI/AI 등은 구축 중.
- 권장: 프로토타입 단계에서는 성능을 고려해 품질 옵션은 보수적으로 설정(후속 튜닝 예정).

### 무기 시스템 설계 초안
- `WeaponBase`(AActor): 데미지, RPM, 탄창/예비탄(종류), 상태(Idle/Firing/Reloading), `Fire()`, `Reload()`, `CanFire()` API.
- `WeaponComponent`(UActorComponent): 소유자 입력을 받아 발사/장전 트리거, 타이머 기반 연사, 장착/해제 관리.
- 히트스캔 흐름: `GetMuzzleTransform` → `LineTraceSingleByChannel` → Hit 데이터로 임팩트 이펙트/사운드, 대미지 적용.
- UI 연동: `WeaponComponent` 이벤트로 탄수/장전 진행을 UMG HUD 바인딩.
- 확장: 발사 방식(Hitscan/Projectile), 반동/스프레드 모델, 임팩트 SFX/VFX 브리지, 네트워크 RPC 등.

## 1~2일 추가 아이디어(Helldivers 느낌 확장)
- 간단 AI: `BehaviorTree`/`EQS`로 추격/공격, AIController + NavMesh.
- 적 폰: 간단/거리 기반 AI 폰 스켈레톤.
- 무기: 무기별 발사속도/반동/스프레드, 애님 슬롯 구조 기초.
- 카메라/콜리전: 좁은 공간에서의 카메라 충돌/당김 부드럽게.
- 체력/목표/HUD: 체력/목표 간단 구현, 저체력 HUD 표시.

## 참고/메모
- 기본 맵 교체: `Config/DefaultEngine.ini`의 `[/Script/EngineSettings.GameMapsSettings] GameDefaultMap`을 `ProtoMap`으로 설정.
- `EnhancedInput` 에셋 구조 추천: `Input/IMC_Default`, `Input/IA_Move`, `Input/IA_Look`, `Input/IA_Fire`, `Input/IA_Sprint`, `Input/IA_Jump`, `IA_Aim`, `IA_Reload`.
- 코드 베이스 위치: `Source/ForSuperDemocracy/` 아래 `Character`, `PlayerController`, `GameMode`, `Weapon` 관련 클래스로 분리 유지.

## 다음 세션 체크리스트
- Unreal Editor에서 `ForSuperDemocracy.uproject` 오픈 후 `ProtoMap`이 기본 맵인지 확인.
- `Content/Input` 폴더의 IMC/IA 에셋과 캐릭터 BP/CPP 연결 상태 확인.
- 작업 진행 상황을 본 문서에 수시 갱신.

---
- 마무리: 자동 작성(Codex CLI)

## 작업 로그 (2025-09-04)
- 장전/재장전 로직 및 발사 속도(RPM) 조정 완료.
- 임시 크로스헤어(UMG) 위젯 제작 및 HUD에 적용.
- 조준/비조준 상태 추가: 조준 상태에서만 발사 가능하도록 게이팅.
- 조준 중 줌(FOV 확대) 적용: 비조준↔조준 전환 시 보간 기반 확대/복귀.

## 내일 할 일 (2025-09-05)
- HealthComponent(UActorComponent) 제작: 체력/피격/사망 이벤트와 대미지 처리(AnyDamage/PointDamage 바인딩), Weapon 대미지 흐름 연결.
- 조준 시 총구 방향 정합: 카메라 시선 기준으로 라인트레이스/발사 방향 보정, 머즐 VFX와 탄착 지점 일치.

