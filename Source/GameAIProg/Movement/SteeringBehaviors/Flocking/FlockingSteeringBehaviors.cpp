#include "FlockingSteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
	Agent.DebugLineFrom(Target.Position, FColor::Cyan);
	Agent.DebugPoint(Target.Position, FColor::Orange);
	
	return Seek::CalculateSteering(deltaT, Agent);
}

SteeringOutput Separation::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Output;
	Output.LinearVelocity = Target.LinearVelocity;
	
	Agent.DebugLineRelative(Target.LinearVelocity, FColor::Red);
	
	return Output;
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
