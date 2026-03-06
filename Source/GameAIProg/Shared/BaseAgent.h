// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BaseAgent.generated.h"

/*
 * Provides common API for editing agent movement characteristics which are almost all held by
 * the character's CharacterMovementComponent. Feel free to directly use its API instead :)
 *
 * All Game AI character will inherit from this class.
 */

UCLASS()
class GAMEAIPROG_API ABaseAgent : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY()
	float OriginalMaxLinearSpeed;

public:
	// Sets default values for this character's properties
	ABaseAgent();

protected:
	bool bIsDebugRenderingEnabled{true};
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// BaseAgent Interface
	FVector2D GetPosition() const { return FVector2D{GetActorLocation().X, GetActorLocation().Y}; }
	FVector2D GetForward() const { return FVector2D{GetActorForwardVector().X, GetActorForwardVector().Y}; }
	float GetRotation() const { return GetActorRotation().Yaw; }
	
	float GetMaxLinearSpeed() const { return GetCharacterMovement()->GetMaxSpeed(); }
	void SetMaxLinearSpeed(float MaxSpeed) { GetCharacterMovement()->MaxWalkSpeed = MaxSpeed; }
	void ResetMaxLinearSpeed() { GetCharacterMovement()->MaxWalkSpeed = OriginalMaxLinearSpeed; }
	
	float GetOriginalMaxLinearSpeed() const noexcept {return OriginalMaxLinearSpeed;}

	FVector2D GetLinearVelocity() const { return FVector2D{GetCharacterMovement()->Velocity}; }
	
	[[nodiscard]]
	double GetLinearSpeed() const
	{
		return GetLinearVelocity().Length();
	}

	float GetMaxAngularSpeed() const { return GetCharacterMovement()->RotationRate.Yaw; }
	void SetMaxAngularSpeed(float maxAngularSpeed) { GetCharacterMovement()->RotationRate.Yaw = maxAngularSpeed; }

	float GetAngularVelocity() const { return GetCharacterMovement()->GetLastUpdateRotation().Yaw - GetActorRotation().Yaw; }

	bool IsAutoOrienting() const { return GetCharacterMovement()->bOrientRotationToMovement; }
	void SetIsAutoOrienting(bool bAutoOrient) { GetCharacterMovement()->bOrientRotationToMovement = bAutoOrient; }

	float GetMass() const { return GetCharacterMovement()->Mass; }
	void SetMass(float Mass) { GetCharacterMovement()->Mass = Mass; }

	bool GetDebugRenderingEnabled() const { return bIsDebugRenderingEnabled; }
	void SetDebugRenderingEnabled(bool IsEnabled) { this->bIsDebugRenderingEnabled = IsEnabled; }
	
	[[nodiscard]]
	FVector ToDebugDrawVector(FVector Vec) const
	{
		return FVector{Vec.X, Vec.Y, GetActorLocation().Z - 50.f};
	}
	
	[[nodiscard]]
	FVector ToDebugDrawVector(FVector2D Vec) const
	{
		return FVector{Vec.X, Vec.Y, GetActorLocation().Z - 50.f};
	}
	
	void DebugLine(FVector2D From, FVector2D To, FColor Color) const;
	
	void DebugLineFrom(FVector2D To, FColor Color) const;
	void DebugLineRelative(FVector2D Vec, FColor Color) const;
	
	void DebugCircle(FVector2D At, float Radius, FColor Color) const;
	void DebugCircleFrom(float Radius, FColor Color) const;
	
	void DebugPoint(FVector2D At, FColor Color) const;
	
	void DebugText(FVector2D At, FColor Color, FString const &Text) const;
};
