#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
// TODO: Do the Week01 assignment :^)
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
