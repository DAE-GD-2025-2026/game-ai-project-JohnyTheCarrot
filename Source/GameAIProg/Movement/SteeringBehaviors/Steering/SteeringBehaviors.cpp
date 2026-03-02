#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	auto const ToTargetVector{[this, &Agent]
	{
		UE::Math::TVector2 DeltaPos{Target.Position.X - Agent.GetActorLocation().X, Target.Position.Y - Agent.GetActorLocation().Y};
		DeltaPos.Normalize();
		return DeltaPos;
	}()};
	
	SteeringOutput Result{};
	Result.LinearVelocity = ToTargetVector;
	
	return Result;
}

void Seek::DebugRender(SteeringOutput const &Output, ASteeringAgent const& Agent) const
{
	DrawDebugLine(
		Agent.GetWorld(), 
		Agent.GetActorLocation(), 
		Agent.GetActorLocation() + UE::Math::TVector{Output.LinearVelocity, 0.},
		FColor::Blue);
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	auto Seek = Seek::CalculateSteering(DeltaT, Agent);
	Seek.LinearVelocity = -Seek.LinearVelocity;
	
	return Seek;
}

void Flee::DebugRender(SteeringOutput const &Output, ASteeringAgent const& Agent) const
{
}

SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	Target.Position = [this, &Agent]
	{
		auto Pos = Agent.GetActorLocation() + Agent.GetActorForwardVector() * Distance;
		Pos += FMath::VRand() * Radius;
		
		return FVector2D{Pos.X, Pos.Y};
	}();
	
	return Seek::CalculateSteering(DeltaT, Agent);
}

void Wander::DebugRender(SteeringOutput const& Output, ASteeringAgent const& Agent) const
{
	FRotator const DesiredAngle{90, 0, 0};
	FVector const YVector = DesiredAngle.RotateVector(FVector(0, 1, 0));
	FVector const ZVector = DesiredAngle.RotateVector(FVector(0, 0, 1));
	
	DrawDebugCircle(
		Agent.GetWorld(),
		Agent.ToDebugDrawVector(Agent.GetActorLocation() + Distance * Agent.GetActorForwardVector()),
		Radius,
		30,
		FColor::Yellow,
		false,
		-1, 0, 0,
		YVector, ZVector
	);
	DrawDebugPoint(
		Agent.GetWorld(),
		Agent.ToDebugDrawVector(FVector2D{Target.Position.X, Target.Position.Y}),
		8.f,
		FColor::Emerald
	);
}
