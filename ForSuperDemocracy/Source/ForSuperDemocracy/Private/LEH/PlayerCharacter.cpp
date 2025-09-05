// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "LEH/PlayerFSM.h"
#include "OSC/Weapon/WeaponComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	
	Camera =CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	ChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActor"));
	ChildActor->SetupAttachment(GetMesh());
	
	FSMComp = CreateDefaultSubobject<UPlayerFSM>(TEXT("FSMComponent"));
	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	
	ConstructorHelpers::FObjectFinder<USkeletalMesh> BodyMeshTemp(TEXT("/Script/Engine.SkeletalMesh'/Game/PROJECTS/HELLDIVERS_2/CHARACTERS/PLAYER/B-01_TACTICAL/fix_v2/SKM_B-01_v1_BRAWNY_SIMPLE.SKM_B-01_v1_BRAWNY_SIMPLE'"));
	if (BodyMeshTemp.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(BodyMeshTemp.Object);
	}
	
	GetMesh()->SetRelativeLocation(FVector(0.000000,0.000000,-83.103748));
	GetMesh()->SetRelativeRotation(FRotator(0.000000,-89.999999,0.000000));
	
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

