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
- 확장: 발사 방식(Hitscan/Projectile), 반동/스프레드 모델, SFX/VFX 브리지.

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

## 작업 로그 (2025-09-08)
- 총기 발사 애니메이션 적용: AHitscanWeapon에서 AnimationSingleNode로 FireAnim 재생 후 원상 복구 타이머 처리.
- 무기 분리: 히트스캔/프로젝타일 무기 클래스를 분리(AHitscanWeapon / AProjectileWeapon), 공통 로직은 WeaponBase 유지.
- 수류탄 구현: AGrenade(USphereComponent + UProjectileMovementComponent) 추가, FuseTime(기본 2.0s) 후 UGameplayStatics::SpawnEmitterAtLocation으로 폭발 파티클 스폰 및 Destroy() 자멸 구현.
- 빌드/종속성: Niagara 제거, Cascade ParticleSystem 사용 경량화.
- 후속 예정: AProjectileWeapon::FireOnce에서 Grenade 스폰 및 초기 속도/오너 속도 상속/오너 충돌 무시, 예측 궤적(PredictProjectilePath) 적용 및 HUD 표기 연동.

## 작업 로그 (2025-09-10)
- 메인 모드/HUD: `AMainMode`, `AMainHUD` 추가. `AMainHUD::BeginPlay`에서 `WBP_MainUI`를 생성해 뷰포트에 추가하여 UI 기초 구축.
- 사격 이펙트: 히트스캔 무기 머즐 `ShotVFX`(Niagara) 스폰, 머즐 소켓 정렬 및 선택적 회전 오프셋 적용.
- 폭발 이펙트: 수류탄 폭발 시 `ExplosionVFX`(Niagara) 스폰. 퓨즈 타이머 만료 시 자동 실행.

## 작업 로그 (2025-09-11)
- 수류탄: `AGrenade::Explode()`에 `UGameplayStatics::ApplyRadialDamage` 적용(반경 데미지). 퓨즈 타이머 만료 후 폭발, `DrawDebugSphere`로 데미지 반경 시각화, `ExplosionVFX`(Niagara) 스폰.
- 무기 UI: `UWeaponWidget` 구현(텍스트 바인딩: `CurrentAmmo/MaxAmmo/CurrentGrenade/MaxGrenade`). `AMainHUD`에서 `UMainUI` 생성·뷰포트 추가, `AWeaponBase::ShowBullet()`에서 현재 탄/예비탄을 HUD 경유로 갱신.
- 무기 베이스/컴포넌트: `AWeaponBase`에 연사 타이밍/재장전 타이머/반동·블룸 누적·회복 로직 추가. `UWeaponComponent`가 ChildActor 무기 자동 등록, 탄약 풀(`PullAmmo`/`GetReserveAmmo`) 관리, ADS 게이팅, 장비 전환 및 사격·재장전 위임.
- 히트스캔/프로젝타일: `AHitscanWeapon` 라인트레이스(커스텀 채널 `Hitscan`) + 포인트 데미지 적용, 발사 애니메이션(AnimSingleNode) 재생 후 원상 복구, 머즐 VFX 스폰. `AProjectileWeapon`은 `AGrenade` 스폰, 초기 속도 부여, 오너 충돌 무시 처리.
- 플레이어/컨트롤: `ASuperPlayerController` Enhanced Input 바인딩(이동/보기/질주/조준/발사/재장전). 조준 시 `APlayerCharacter::StartZoom`으로 FOV 보간 및 이동속도 조정, ADS 상태에서만 사격 허용, 조준 완료 시 크로스헤어 표시.
- 몬스터/체력: `UHealthComponent`가 Any/Point/Radial 데미지 바인딩과 체력 변화/사망 이벤트 브로드캐스트 구현. `ATerminidBase`가 `OnDamaged` 연결로 피격시 타겟 설정 등 FSM 연계 기반 마련.

### 후속 예정/주의
- 게임 시작 시 탄약 UI 초기화 루틴 추가(무기 등록 직후 1회 갱신 필요).
- 수류탄 보유량 UI(`SetGrenade/InitWeapons`) 실제 데이터와 연결.
- `ATerminidBase::BeginPlay()`에서 `Health` null 체크 보강(컴포넌트 미부착 크래시 방지).
- HUD/GameMode 설정에서 `AMainHUD` 지정 상태 확인.

---

## 현황 분석 (2025-09-12 ~ 2025-09-15)

### 요약
- UI: 체력 UI(HUD) 실제 데미지 이벤트 연동 완료. `OnFired`/`WeaponFire` 델리게이트로 HUD 갱신 신뢰성 강화.
- 무기/전투: 히트스캔에 더해 `ProjectileBase/ProjectileWeapon/Grenade` 경로 신설(투사체 계열 도입). 래그돌 적용 지점 추가.
- 애니메이션: Fire/Reload 애님 및 경례 몽타주 연동 완료. 스캐빈저 걷기/공격 시퀀스 보강, AnimNotify로 데미지 트리거 처리.
- AI/레벨: 스포너 충돌/배치 정리 및 암석/바위 콜리전 값 보정. 레벨(바위 아치 등) 배치 품질 향상.
- 자산/구조: 대규모 경로 재정리(.gitignore 포함). 플레이어 망토 도입 및 콜리전/물리 설정 보정.

### 통합 상태
- HUD/체력:
  - `UHealthWidget`가 데미지 이벤트와 연동되어 체력 표시 갱신됨.
  - 무기 발사 이벤트(`OnFired`/`WeaponFire`)를 UI와 연결해 탄약/발사 상태 갱신 루틴 정착.
- 무기 시스템:
  - 히트스캔 ↔ 투사체 이원화. `ProjectileBase`를 통해 그레네이드/프로젝트타일 무기 계열 확장 준비 완료.
  - 발사/재장전 애님(AM_Fire/AM_Reload)과 데이터(`OSC/Data/Rifle`) 연동 확인.
- 애니메이션/캐릭터:
  - 경례 애니메이션(몽타주) 도입 및 완료 처리.
  - 스캐빈저 이동/공격 애니메이션 적용. 공격 타이밍은 AnimNotify 기반으로 데미지 처리.
  - 플레이어 망토 추가 및 콜리전/물리 값 보정으로 클리핑 최소화.
- 레벨/프롭/콜리전:
  - 스포너 콜리전, 암석 콜리전, 바위 아치 배치 등 레벨 품질 개선 작업 반영.
  - 대량 자산 경로 이동으로 레퍼런스 리다이렉터 정리 필요성 상존.
- 설정/구성:
  - `Default*.ini` 일부 업데이트 및 `.gitignore` 보강. 자산 경로 체계(FSD/ → ForSuperDemocracy/ 등) 정돈.

### 리스크/주의
- 자산 경로 대량 이동:
  - UE 리다이렉터 정리(Fix Up Redirectors) 필요. 누락 시 레벨/블루프린트 참조 깨짐 가능.
  - CI/빌드 스크립트에서 경로 변경 반영 여부 확인.
- AnimNotify 데미지 처리:
  - Notify는 트리거 신호만 사용하고, 실제 판정은 코드(Trace/Overlap)에서 1회만 수행하도록 가드. 멀티히트(프레임 중복) 방지 타이밍 검증 권장.
- 투사체 데미지 흐름:
  - `Any/Point/RadialDamage` 케이스 모두 `UHealthComponent`와 일관되게 연결되는지 점검.
- 래그돌/물리:
  - 프레임 안정성/퍼포먼스 영향 검토 필요(활성 개수 상한/시간 제한 등 가드레일 설정).
- 망토(클로스/콜리전):
  - 관통/떨림 현상 최소화 튜닝 지속 필요. 상태 전환(스프린트/ADS) 시 물리 값 전환 타이밍 점검.

### 완료 체크(최근)
- [완료] Health UI ↔ 데미지 이벤트 연동(2025-09-12)
- [완료] Fire/Reload 애니메이션 연계(2025-09-12)
- [완료] 경례 애니메이션/몽타주 도입(2025-09-12~14)
- [완료] ProjectileBase/ProjectileWeapon/Grenade 도입(2025-09-12)
- [완료] Ragdoll + Stratagem 시스템 스캐폴딩(코드/자원 반영)(2025-09-14)
- [완료] Scavenger 애니메이션/AnimNotify 데미지 처리(2025-09-14)
- [완료] 스포너/레벨 콜리전/배치 정리(2025-09-14)
- [완료] 자산 경로 구조 재정리 및 `.gitignore` 업데이트(2025-09-15)
- [완료] 플레이어 망토 추가 및 콜리전 보정(2025-09-14~15)

### 후속 TODO (2025-09-15)
- [레벨/자산] Redirectors 정리 실행 및 참조 검증(레벨/BP/애님BP 전수 확인).
- [전투/투사체] RadialDamage/파편 데미지 HUD 반영, 팀/아군 피해 정책 단일화.
- [Stratagem] 입력(시퀀스/쿨다운)/UI(요청/상태) 설계 및 구현.
- [AI] Scavenger 데미지 Notify의 단일 틱 내 다중 히트 방지 로직 추가.
- [물리] 래그돌/망토 퍼포먼스(프레임 타임/GC) 측정 및 튜닝, 재현성 테스트.
- [UI] 무기/체력/UI 초기화 타이밍 단일화(Spawn/BeginPlay 시 1회 동기화 루틴).

---

## 주간 계획 (2025-09-15 주)

### 우선순위(P0)
- 플레이어: 엎드리기(Prone) 구현, 피격 이벤트 반응/연출 통합
- 적: 종류 추가(최소 워리어/차저), 스폰/웨이브 연동
- 임무: 기본 목표/상태 기계(Kill/Survive 중심) 및 UI
- 플로우: 게임 시작 → 플레이 → 추출 → 결과 → 재시작 흐름

### 팀 배분(3인)
- Dev A — 플레이어: 엎드리기/피격/카메라/UI 연동
- Dev B — 적/스폰: 워리어·차저 타입, 공격 판정, 스폰/웨이브
- Dev C — 임무/플로우: 미션 컴포넌트·UI, 추출/결과, 게임 상태 전환

### 공유 인터페이스(합의 필수)
- 데미지 이벤트: `UHealthComponent::OnDamaged(AActor* Causer, float Damage, FVector HitLoc)`
- 근접 판정: `bool ApplyMeleeHit(const FHitEvent& E)` — 한 번만 판정(쿨다운 윈도우 포함)
- 미션 스펙: `struct FMissionSpec { EObjectiveType Type; int32 Target; float TimeLimit; }`
- 브로드캐스트: `OnObjectiveUpdated(int32 Curr,int32 Target)`, `OnMissionCompleted()`, `OnMissionFailed()`
- 스폰→미션: 적 사망 시 `UMissionComponent::NotifyKill(EEnemyType Type)` 호출

### 구현 항목 상세
- Dev A — 플레이어(엎드리기·피격)
  - 입력/상태: `IA_Prone` 추가, `UPlayerFSM`에 `Prone` 상태(Enter/Update/Exit)
  - 캡슐/카메라: HalfHeight 전환, 스프링암 높이/FOV 보정, ADS 제약 정책
  - 애님: `ABP_PlayerAnimInstance` Prone 전이/Blend, Prone Idle/Move 연결
  - 피격: `OnDamaged` 수신→ HitReact additive, 카메라 셰이크/히트 플래시, 무적 윈도우 상수화
- Dev B — 적 타입/스폰
  - 데이터: `UTerminidDataAsset`(HP/Speed/AtkRate/Damage/HitReact)
  - 클래스: `ATerminidWarrior`, `ATerminidCharger` 파생(이동 패턴·근접 공격)
  - 공격: AnimNotify→코드 판정 일원화(중복 히트 가드), `ApplyMeleeHit` 활용
  - 스폰: `TerminidSpawner`에 타입/수량/쿨다운/웨이브 정의, 테스트 웨이브 구성
- Dev C — 임무/플로우
  - 미션: `UMissionComponent`/`AMissionManager`(GameMode 종속), 이벤트 브로드캐스트
  - UI: `WBP_MainUI` 목표 패널(진척/타이머), 완료/실패 토스트
  - 플로우: `AMainMode` 상태(PreGame→Playing→Extraction→Debrief→Menu)와 트리거
  - 추출: `BP_ExtractionPoint` 배치→ 도달 시 Debrief→ 결과 화면 → 재시작

### 권장 진행 순서(5일)
- 월: Dev A 엎드리기 전이/카메라, Dev B 데이터자산/워리어 프로토, Dev C 미션 스펙/컴포넌트 골격
- 화: Dev A 피격 루프, Dev B 차저/공격 판정 일원화, Dev C 목표 UI/타이머
- 수: Dev B 스폰/웨이브 → 미션 Kill 연동, Dev C PreGame→Playing 전환
- 목: Dev C 추출/결과/재시작, 통합 리허설 1차
- 금: 폴리시 정리(팀피해/ADS·Prone 제약), 버그픽스/튜닝/최종 데모

### 통합 체크포인트
- 공통 시그니처 확정: `OnDamaged`, `NotifyKill`, `FMissionSpec` — D1 12:00까지
- 리허설: D3 EOD(스폰→Kill 집계→UI), D4 EOD(전체 플로우)

### 리스크/완화
- 충돌/지형: Prone 캡슐 전환 시 경사/문턱 테스트 맵으로 검증
- 중복 히트: 공격 ID/쿨다운 윈도우로 단일 판정 보장
- 스코프: 이번 주는 `Kill/Survive` 중심, `Collect/Defend`는 훅만 노출

### 브랜치/PR
- 브랜치: `feat/player-prone-and-hit/devA`, `feat/enemy-types-and-spawn/devB`, `feat/mission-and-flow/devC`
- 규칙: 작은 PR(≈300 라인 코드), 머지 전 `Save All` 및 `Fix Up Redirectors` 확인, 로컬 UBT 스모크 빌드

## 작업 로그 (2025-09-16)
- 미션 데이터/컴포넌트 골격 완성(싱글 기준): `UMissionData`(DataAsset) + `UMissionComponent`(ActorComponent).
- 미션 타입/스펙 정리: `EMissionType { Kill, Destroy, ReachArea, Survive }`, `FMissionObjective { Type, Target, TimeLimit, FilterTag, TargetClass, AreaTag, Label }`.
- 컴포넌트 핵심 로직 구현:
  - `StartMission` → 첫 Objective 시작, 상태 초기화.
  - `StartObjective` → 진행도/타이머 초기화, Survive/TimeLimit 1Hz 카운트다운.
  - `TickTimer` → Survive 완료/제한시간 만료 시 실패 처리.
  - `NotifyKill/NotifyDestroyed/NotifyAreaEntered` → 타입별 진행/완료, Destroy 중복 집계 가드.
  - `CompleteObjective/FailObjective` → 다음 Objective 전환 또는 미션 완료/실패 브로드캐스트.
- 이벤트 델리게이트 노출: `OnObjectiveChanged/Updated/Completed`, `OnMissionCompleted/Failed`, `OnTimerTick`(UI 연동용).
- MainMode 연동: `UMissionComponent` UPROPERTY 보유, `BeginPlay()`에서 `StartMission()` 호출(블루프린트에서 `UMissionData` 지정, 이벤트 훅 BP 연결 중).

## 일일 TODO (2025-09-17)
- 미션 UI 구현(WBP_MainUI 내 Mission 패널): 제목(Label/Type), 진행도(`Curr/Target` + ProgressBar), 타이머(mm:ss), 상태 배지.
- 델리게이트 바인딩: `OnObjectiveChanged/Updated/TimerTick/Completed/Failed` 수신 → 위젯 갱신/토스트 1회 표시.
- 초기 동기화: UI 생성 직후 현재 Objective/진행도/타이머 값 즉시 반영(깜빡임 방지).
- Survive 연동: 시작 시 웨이브 Start, 완료/실패 시 Stop(중복 호출 가드).
- ReachArea 표시: 복귀 지점 월드 마커/스크린 핀(간단 버전) 추가.
- 가드 케이스: Target<=0 즉시완료, 중복 이벤트 방지(둥지 1회 집계), 타이머 일시정지 정책 확인.
