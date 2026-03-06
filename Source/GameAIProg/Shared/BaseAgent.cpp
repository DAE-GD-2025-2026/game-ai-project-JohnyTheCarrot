// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAgent.h"


// Sets default values
ABaseAgent::ABaseAgent()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void ABaseAgent::BeginPlay()
{
	Super::BeginPlay();
	OriginalMaxLinearSpeed = GetMaxLinearSpeed();
}

// Called every frame
void ABaseAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABaseAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABaseAgent::DebugLine(FVector2D From, FVector2D To, FColor Color) const
{
	if (bIsDebugRenderingEnabled)
		DrawDebugLine(
			GetWorld(), 
			ToDebugDrawVector(From),
			ToDebugDrawVector(To),
			Color
		);
}

void ABaseAgent::DebugLineFrom(FVector2D To, FColor Color) const
{
	DebugLine(GetPosition(), To, Color);
}

void ABaseAgent::DebugLineRelative(FVector2D Vec, FColor Color) const
{
	DebugLineFrom(GetPosition() + Vec, Color);
}

void ABaseAgent::DebugCircle(FVector2D At, float Radius, FColor Color) const
{
	if (!bIsDebugRenderingEnabled) return;
	
	FRotator const DesiredAngle{90, 0, 0};
	FVector const YVector = DesiredAngle.RotateVector(FVector(0, 1, 0));
	FVector const ZVector = DesiredAngle.RotateVector(FVector(0, 0, 1));
	
	DrawDebugCircle(
		GetWorld(),
		ToDebugDrawVector(At),
		Radius,
		30,
		Color,
		false,
		-1, 0, 0,
		YVector, ZVector
	);
}

void ABaseAgent::DebugCircleFrom(float Radius, FColor Color) const
{
	DebugCircle(GetPosition(), Radius, Color);
}

void ABaseAgent::DebugPoint(FVector2D At, FColor Color) const
{
	if (!bIsDebugRenderingEnabled) return;
	
	DrawDebugPoint(GetWorld(), ToDebugDrawVector(At), 10.f, Color);
}

void ABaseAgent::DebugText(FVector2D At, FColor Color, FString const& Text) const
{
	if (!bIsDebugRenderingEnabled) return;
	
	DrawDebugString(GetWorld(), ToDebugDrawVector(At), Text, nullptr, Color, 0);
}
