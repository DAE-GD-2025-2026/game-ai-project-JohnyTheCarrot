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
	Result.LinearVelocity = ToTargetVector * Agent.GetMaxLinearSpeed();
	
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
