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
	
	Agent.DebugLineRelative(Result.LinearVelocity, FColor::Yellow);
	
	return Result;
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	auto Seek = Seek::CalculateSteering(DeltaT, Agent);
	Seek.LinearVelocity = -Seek.LinearVelocity;
	
	return Seek;
}

SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	auto const CircleCenter = Agent.GetPosition() + Agent.GetForward() * Distance;
	auto const RandVec3d = FMath::VRand().GetSafeNormal2D();
	Target.Position = CircleCenter + FVector2D{RandVec3d.X, RandVec3d.Y} * Radius;
	
	Agent.DebugCircle(CircleCenter, Radius, FColor::Yellow);
	Agent.DebugLineFrom(Target.Position, FColor::Emerald);
	
	return Seek::CalculateSteering(DeltaT, Agent);
}

