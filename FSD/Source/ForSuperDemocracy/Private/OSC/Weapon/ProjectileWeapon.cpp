// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Weapon/ProjectileWeapon.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "OSC/Weapon/Grenade.h"
#include "OSC/Weapon/ProjectileBase.h"

void AProjectileWeapon::FireOnce()
{
	if (!CanFire())
		return;
	Super::FireOnce();

	if (!ProjectileClass) return;
	
	const FVector SpawnLoc = GetMuzzleLocation();                                                                        
	FRotator ViewRot = GetFireRotation();
	
	FActorSpawnParameters Params;                                                                                        
	Params.Owner = GetOwner();                                                                                           
	Params.Instigator = Cast<APawn>(GetOwner());                                                                         
	
	AActor* Proj = GetWorld()->SpawnActor<AActor>(                                                                       
		 ProjectileClass, FTransform(ViewRot, SpawnLoc), Params);

	if (!Proj) return;

	if (UProjectileMovementComponent* PM = Proj->FindComponentByClass<UProjectileMovementComponent>())                   
	{                                                                                                                    
		PM->Velocity = ViewRot.Vector() * InitialSpeed;                                   
		PM->Activate(true);                                                                                              
	}
	
	if (UPrimitiveComponent* Col = Cast<UPrimitiveComponent>(Proj->GetRootComponent()))                                  
	{                                                                                                                    
		if (AActor* OwnerActor = GetOwner())                                                                             
			Col->IgnoreActorWhenMoving(OwnerActor, true);                                                                
	}  
}
