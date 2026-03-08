
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include <ranges>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(std::initializer_list<WeightedBehavior> WeightedBehaviors)
	: WeightedBehaviors{WeightedBehaviors}
{
}

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	
	for (const auto [pBehavior, Weight] : WeightedBehaviors)
	{
		if (Weight == 0.f) continue;
		
		auto const Steering = pBehavior->CalculateSteering(DeltaT, Agent);
		BlendedSteering.LinearVelocity += Steering.LinearVelocity.GetSafeNormal() * Weight;
	}
	
	// TODO: Calculate weighted sum for angle, too
	
	Agent.DebugLineRelative(BlendedSteering.LinearVelocity, FColor::Purple);

	return BlendedSteering;
}

float* BlendedSteering::GetWeight(ISteeringBehavior const* SteeringBehavior)
{
	auto const Iter = std::ranges::find_if(WeightedBehaviors,
		[SteeringBehavior](WeightedBehavior const &Elem)
		{
			return Elem.pBehavior_ == SteeringBehavior;
		}
	);

	if (Iter != WeightedBehaviors.cend())
		return &Iter->Weight_;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If none of the behavior return a valid output, last behavior is returned
	return Steering;
}
