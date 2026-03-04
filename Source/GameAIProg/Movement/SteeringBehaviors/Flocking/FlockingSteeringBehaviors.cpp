#include "FlockingSteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	return Seek::CalculateSteering(deltaT, pAgent);
}

void Cohesion::DebugRender(SteeringOutput const& Output, ASteeringAgent const& Agent) const
{
	DrawDebugLine(
		Agent.GetWorld(),
		Agent.ToDebugDrawVector(Agent.GetActorLocation()),
		Agent.ToDebugDrawVector(Target.Position),
		FColor::Cyan, false, -1, 0, 3.f
	);
	DrawDebugPoint(
		Agent.GetWorld(),
		Agent.ToDebugDrawVector(Target.Position),
		7.f, FColor::Orange
	);
}

SteeringOutput Separation::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Output;
	Output.LinearVelocity = Target.LinearVelocity;
	return Output;
}

void Separation::DebugRender(SteeringOutput const& Output, ASteeringAgent const& Agent) const
{
	// DrawDebugLine(
	// 	Agent.GetWorld(),
	// 	Agent.GetTransform().GetLocation(),
	// 	Agent.GetTransform().GetLocation() + FVector{Target.LinearVelocity.X, Target.LinearVelocity.Y, 0.f},
	// 	FColor::Red, false, -1, 0, 4.f
	// );
}

SteeringOutput Alignment::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Output;
	Output.LinearVelocity = Target.LinearVelocity;
	return Output;
}

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)
