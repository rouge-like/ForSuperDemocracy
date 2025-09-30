# For Super Democracy

## 프로젝트 개요
- **장르**: 협동형 TPS 프로토타입 (Helldivers 2 영감)
- **엔진**: Unreal Engine 5.6 (Include Order 5.6)
- **핵심 목표**: 빠른 전투 루프·미션 플로우·팀 플레이 HUD를 갖춘 플레이어블 슬라이스 제작
- **팀 구성**: 3명 (플레이어 / 적·스폰 / 미션·플로우·전투 담당)

## 게임플레이 루프 요약
1. 로비 없이 `ProtoMap`에서 즉시 전투 시작
2. 플레이어는 기본 무기·그레네이드·스킬을 활용해 적 웨이브와 교전
3. 미션 목표(Kill → Destroy → ReachArea → Survive)를 순차 수행하며 HUD로 진행 상황 확인
4. 최종 목표 완료 시 시네마틱 UI 전환 및 결과 정리

## 역할별 기여

### 플레이어 시스템 담당 (LEH)
- **캐릭터 파이프라인**: `LEH/PlayerCharacter` 중심으로 이동, 대시, 슬라이드, 엎드리기(Prone) 등 전투 기동 구현.
- **상태 머신**: `UPlayerFSM`을 사용해 Idle/Move/Prone/HitReact/ADS 전이를 관리하고, 애님 블렌딩·카메라 제약을 연동.
- **카메라 & 입력**: SpringArm 충돌 보정, 조준 FOV 보간, Enhanced Input(IA_Move/Look/Sprint/Aim/Fire) 매핑 설계.
- **UI 연동**: 체력·스태미나·재장전 상태를 HUD에 전달하고, 플레이어 히트 리액션·카메라 셰이크 트리거.

### 적 & 스폰 담당 (Huxley)
- **적 타입**: `Huxley/Enemy` 계층에서 Terminid Warrior·Charger 클래스 제작, 이동 AI·근접 공격 판정을 공통 함수(`ApplyMeleeHit`)로 통합.
- **데이터 자산**: `UTerminidDataAsset`으로 HP, 이동속도, 공격 주기, 데미지, 히트 리액션 설정을 데이터 드리븐으로 관리.
- **스폰 & 웨이브**: `TerminidSpawner`가 웨이브 패턴, 수량, 쿨다운을 정의하고, 미션 컴포넌트 이벤트(`NotifyKill`)와 연동.
- **네비게이션**: EQS/BehaviorTree 기반 추격, 회피, 집단 이동 패턴 프로토타입.

### 미션 · 플로우 · 전투 시스템 담당 (OSC)
- **미션 오케스트레이션**: `UMissionData`와 `UMissionComponent`가 Kill / Destroy / ReachArea / Survive 순서를 정의하고, 타이머·중복 방지(TSet)·태그/클래스 필터 기반 집계를 수행.
- **HUD & Compass**: `AMainHUD` → `UMainUI` → `UMissionWidget` · `UCompassWidget` 체인을 통해 목표 변경·진행도·거리/방위 나침반을 실시간 갱신. 헤딩 테이프는 `UMaterialInstanceDynamic`으로 제어.
- **전투 코어**: `UWeaponComponent`가 Child Actor 무기를 자동 등록하고, ADS 상태 게이팅·탄약 풀 공유·블룸/반동 회복 로직을 관리. `AWeaponBase`는 실제 발사, HUD 갱신, 반동 적용을 담당.
- **생존성 & 연출**: `UHealthComponent`가 Any/Point/Radial 데미지 이벤트를 수신, 래그돌 전환/복구 시 캡슐·메시·카메라 싱크를 유지하며 미션 완료 시 무적 플래그로 게임 종료 연출을 보조.
- **UI 위젯 모듈**: `UMainUI`, `UCompassEntry`, `UWeaponWidget`, `UHealthWidget` 등이 이벤트를 구독해 핵심 지표를 한 화면에서 제공.

## 기술 스택
- Unreal Engine 5.6
- Enhanced Input 시스템
- Lumen 기반 글로벌 일루미네이션, DX12/Vulkan SM6 타깃
- C++ 모듈 단일 구성 (`ForSuperDemocracy`)

## 개발 기록 요약
- **2025-09-04**: 플레이어 이동/입력 세팅, 무기 RPM·재장전 정비, HUD 임시 위젯, ADS FOV 보간.
- **2025-09-10**: Terminid Warrior AI 프로토타입, 웨이브 스폰 초기 구성.
- **2025-09-16**: 미션 데이터/컴포넌트 골격 완성, 델리게이트 설계, Compass 아트 방향 합의.
- **2025-09-17~18**: Mission UI 연동, Compass 타깃 정렬, HealthComponent 래그돌 처리, 적 히트 리액션 연동.
- **2025-09-22 이후**: WeaponComponent Child Actor 자동 등록, HUD-Weapon 연동 고도화, 미션-웨이브 상호작용 점검.


## 기여자 역할 (3인 팀 기준)
- **플레이어 담당**: 캐릭터 FSM/애님, 카메라·입력 파이프라인, HUD 연계, 플레이어 피격 피드백.
- **적·스폰 담당**: Terminid 데이터·AI·웨이브 시스템, 미션 Kill 연동, 협동 난이도 튜닝.
- **미션·플로우·전투 담당**: OSC 모듈 설계/구현, 무기·체력 코어, Compass/HUD, 게임 진행 상태 관리.

## 시연 영상
https://docs.google.com/presentation/d/1qKJbZ-LUJ3QGyPnaGfR_aRzeU9u565JQhcUfK1U2T88/edit?usp=drive_link
