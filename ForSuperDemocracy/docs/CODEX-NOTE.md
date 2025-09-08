# CODEX NOTE

- 프로젝트 목표: TPS 게임(Helldivers 2 느낌) 프로토타입
- 엔진: Unreal Engine 5 프로젝트(`ForSuperDemocracy.uproject`)

## 일정/진척
- 팀 구성: 3인 개발(가칭)
- 담당(임시): 무기, UI, 시스템
- 오늘 초점: 무기 `WeaponBase`와 `WeaponComponent` 구체화

## 현재 코드/구성 요약
- 엔진/빌드: UE 5.6, 최신 IncludeOrder. `Game`/`Editor` 타깃(`Source/ForSuperDemocracy.Target.cs`, `Source/ForSuperDemocracyEditor.Target.cs`).
- 게임 모듈: `ForSuperDemocracy` 단일 모듈. 기본 모듈 등록/실행 상태.
  - `Source/ForSuperDemocracy/ForSuperDemocracy.cpp`: `FDefaultGameModuleImpl`로 기본 모듈 등록/실행.
  - `Source/ForSuperDemocracy/ForSuperDemocracy.h`: 기본 헤더 포함.
  - `Source/ForSuperDemocracy/ForSuperDemocracy.Build.cs`: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput` 종속.
- 콘텐츠: `Content/Maps/ProtoMap.umap` 존재 예정. `Config/DefaultEngine.ini`의 `GameDefaultMap`은 `ProtoMap`으로 설정됨(에디터에서 확인 권장).
- 입력 설정: `EnhancedInput` 사용 계획(매핑/바인딩 준비 필요).
- 렌더: Windows DX12 + SM6, Linux Vulkan SM6. Lumen(동적 GI/리플렉션) 사용, Nanite/RT는 상황에 따라 조정.

## 현재 상태 진단
- 상태: 초기 스켈레톤 설계. 캐릭터/플레이어 컨트롤러/게임모드/입력 매핑/기본 UI/AI 구축 진행 중.
- 권장: 프로토 설계서 기준으로 기능 가로 슬라이스 보수 및 지속 러닝 설정.

### 무기 시스템 설계 초안
- `WeaponBase`(AActor): 데미지, RPM, 탄창/탄종, 상태(Idle/Firing/Reloading), `Fire()`, `Reload()`, `CanFire()` API.
- `WeaponComponent`(UActorComponent): 소유자 입력 받아 발사/재장전 처리, 타이머 기반 발사, 장착/교체 관리.
- 히트스캔 흐름: `GetMuzzleTransform` + `LineTraceSingleByChannel`, Hit 데이터로 이펙트/사운드/데미지 처리.
- UI 연동: `WeaponComponent` 이벤트로 탄수/재장전 진행 HUD 바인딩.
- 확장: 발사 방식(Hitscan/Projectile), 반동/스프레드 모델, SFX/VFX 브리지, 네트워크 RPC.

## 1~2주 추가 아이디어(Helldivers 느낌 확장)
- 간단 AI: `BehaviorTree`/`EQS` 추격/공격, AIController + NavMesh.
- 군중 간단/거리 기반 AI 스켈레톤.
- 무기: 발사 주기/반동/스프레드, 그립/슬롯 구조 기초.
- 카메라/콜리전: 좁은 공간에서 카메라 충돌/클리핑 부드럽게.
- 체력/목표/HUD: 체력/목표 간단 구현, 파티 체력 HUD 표시.

## 참고/메모
- 기본 맵 교체: `Config/DefaultEngine.ini`의 `[/Script/EngineSettings.GameMapsSettings] GameDefaultMap`을 `ProtoMap`으로 설정.
- `EnhancedInput` 에셋 구조 추천: `Input/IMC_Default`, `Input/IA_Move`, `Input/IA_Look`, `Input/IA_Fire`, `Input/IA_Sprint`, `Input/IA_Jump`, `IA_Aim`, `IA_Reload`.
- 코드 베이스 배치: `Source/ForSuperDemocracy/` 아래 `Character`, `PlayerController`, `GameMode`, `Weapon` 관련 클래스로 분리 유지.

## 다음 액션 체크리스트
- Unreal Editor에서 `ForSuperDemocracy.uproject` 오픈 후 `ProtoMap`이 기본 맵인지 확인.
- `Content/Input` 폴더의 IMC/IA 에셋을 캐릭터 BP/CPP에 연결 상태 확인.
- 작업 진행 상황과 문서를 주기적으로 갱신.

---

## 작업 로그 (2025-09-04)
- 재장전/발사 로직 및 발사 주기(RPM) 조정 완료.
- 임시 UI(HUD/UMG) 위젯 제작 및 HUD 적용.
- 조준/비조준 상태 추가: 조준 상태에서 발사 가중치 적용.
- 조준 시 시야(FOV) 전환: 비조준↔조준 전환 보간 기반 적용/복귀.

## 일일 TODO (2025-09-05)
- HealthComponent(UActorComponent) 제작: 체력/피격/사망 이벤트 및 데미지 처리(AnyDamage/PointDamage 바인딩, Weapon 데미지 흐름 연결).
- 조준 시 총구 방향 정합: 카메라 시선 기반으로 레이캐스트/발사 방향 보정, 머즐 VFX와 일치 지점 유지.

## 리포 점검 요약 (2025-09-06)
- 엔진/빌드: UE 5.6, IncludeOrder 5.6, 단일 모듈 정상. EnhancedInput 종속성 포함.
- 맵/렌더: Default/Editor 맵 `ProtoMap`으로 지정, DX12/Vulkan SM6 + Lumen/RT 설정 반영.
- 시스템: 무기(WeaponBase/WeaponComponent/WeaponData/AmmoType), 체력(HealthComponent) 구현 확인.
- 캐릭터/컨트롤러: PlayerCharacter(SpringArm/Camera/FSM) + SuperPlayerController(Move/Look/Sprint) 기본 구현.
- 통합 미흡: WeaponComponent 캐릭터 부착/입력 바인딩(발사/재장전/조준), ADS 보간/정렬, HUD/UMG 연동, Input 에셋 연결 상태 확인 필요.

## 일일 TODO (2025-09-06)
- 입력 바인딩: 컨트롤러에 IA_Fire/IA_Reload/IA_Aim(시작/종료) 바인딩, WeaponComponent의 StartFire/StopFire/Reload/StartAiming/StopAiming 연결.
- 무기 컴포넌트 통합: PlayerCharacter에 WeaponComponent 추가, ChildActor 무기 자동 등록, 시작 탄약 풀 초기화.
- ADS/카메라: 조준 중 UpdateAimAlignment 호출, FAimViewParams로 카메라(FOV/암 길이/오프셋) 보간 적용.
- HUD 임시화: 체력/탄약 표시용 간단 위젯 또는 디버그 HUD 추가, HealthComponent 이벤트/탄약 표시 연동.
- 맵/에셋 확인: ProtoMap 및 IMC/IA 입력 에셋 존재/링크 점검(없으면 생성/연결).

## 작업 로그 (2025-09-05)
- 46309e9 HealthComponent + ApplyDamage 추가: 체력 컴포넌트(Any/PointDamage 바인딩) 도입, 무기 데미지 흐름 연결, IMC/IA 입력 에셋 경로 재구성, ProtoMap/캐릭터 BP 반영.

## 작업 로그 (2025-09-07)
- 7874cfd NOTE 업데이트 및 반동 초안: 반동/스프레드 데이터·로직 초안 적용, 문서 갱신.
- 964c098 주석 정리: Weapon/Health 관련 헤더·CPP 주석 보강, IMC_Weapon 에셋 미세 업데이트.
- 60bcac1 총기 반동 복구 구현: 즉시 킥 + 틱 기반 서서히 복귀(자연스러운 TPS 감각), Rifle 데이터 튜닝.
- 98a71d9 Merge main→OSC: 충돌 없이 통합.

## 설계 변경 요약(반동/스프레드)
- ADS 전용 사격: 허리사격 비활성, 스프레드/반동 단일 파라미터로 단순화.
- 스프레드/블룸: BaseSpread + CurrentBloom, 발당 증가/초당 회복. 라인트레이스에 VRandCone 적용.
- 반동(카메라): 발사 시 즉시 소량 킥, Tick에서 초당 복구 속도로 원조준에 서서히 복귀.

## 오늘 TODO (2025-09-07)
- 반동 튜닝: 무기별 Pitch/Yaw 범위, 복구 속도(RecoilRecoverDegPerSec) 값 조정 및 플레이 테스트.
- 데이터 정리: 기존 무기 데이터에 BaseSpread/SpreadMax/Recovery 값 적용, 불필요한 Hip/ADS 분기 제거 확인.
- 입력/ADS 게이팅 재확인: ADS 아닐 때 사격 금지 로직 검증(WeaponComponent::StartFire).
- 성능/체감: 디버그 라인/메시 임시 유지로 명중 정렬·복구 체감 점검, 과도시 값 보정.
- 문서/채널 동기화: 깃-관리 요약과 본 문서 로그 일치 유지.

## 내일 TODO (2025-09-08) — 특수무기: 수류탄(Grenade)
- 시스템 설계: 투척 무기 파이프라인 정의(WeaponComponent 확장 또는 GrenadeComponent 별도 구성).
- 입력: `IA_ThrowGrenade` 추가 및 컨트롤러 바인딩(탭: 즉시 투척, 홀드: 조리 Cook 후 투척).
- 데이터: `UGrenadeData`(DataAsset) 설계(fuseTime, radius, damage, projectileSpeed, maxCarry, bCookable, VFX/SFX, DamageType).
- 액터: `AGrenadeProjectile` 구현(StaticMesh/ProjectileMovement 또는 PhysicsSim, 퓨즈 타이머, 충돌 시/퓨즈 만료 시 폭발 → `UGameplayStatics::ApplyRadialDamage`).
- 투척 플로우: 궤적 프리뷰(`UKismetSystemLibrary::PredictProjectilePath`), 소유자 근접 자폭 방지(짧은 무적/IgnoreOwner 시간), 카메라 충돌/안전거리 확보.
- 인벤토리/HUD: 소지 수량(획득/소모), HUD 표기(잔량/쿨다운), 입력 불가 상태 처리.
- 충돌/팀킬: 팀 구분/피해 최소화 옵션, DamageType 세분화(폭발/화염 등)와 저항치 고려.
- 기본 값 제안: radius 350, damage 120, fuse 3.0s, projectileSpeed 1600, maxCarry 2.
- 리플리케이션(후순위): 서버 권위 투척/폭발, 멀티캐스트 VFX/SFX 훅 준비.
