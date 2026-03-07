// Fill out your copyright notice in the Description page of Project Settings.

#include "SteeringAgent.h"


// Sets default values
ASteeringAgent::ASteeringAgent()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASteeringAgent::BeginPlay()
{
	Super::BeginPlay();
}

void ASteeringAgent::BeginDestroy()
{
	Super::BeginDestroy();
}

// Called every frame
void ASteeringAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PrimaryActorTick.bCanEverTick = !IsDirected;
	if (!IsDirected && SteeringBehavior)
	{
		SteeringOutput const Output = SteeringBehavior->CalculateSteering(DeltaTime, *this);
		AddMovementInput(FVector{Output.LinearVelocity, 0.f});
	}
}

// Called to bind functionality to input
void ASteeringAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASteeringAgent::SetSteeringBehavior(ISteeringBehavior *pNewSteeringBehavior)
{
	ResetMaxLinearSpeed();
	SteeringBehavior = pNewSteeringBehavior;
}

void ASteeringAgent::PerformSteer(float DeltaTime)
{
	if (!IsDirected)
		UE_LOG(LogTemp, Warning, TEXT("Agent is being directed by an external source, but is not marked as directed"));
	
	SteeringOutput const Output = SteeringBehavior->CalculateSteering(DeltaTime, *this);
	AddMovementInput(FVector{Output.LinearVelocity, 0.f});
}
