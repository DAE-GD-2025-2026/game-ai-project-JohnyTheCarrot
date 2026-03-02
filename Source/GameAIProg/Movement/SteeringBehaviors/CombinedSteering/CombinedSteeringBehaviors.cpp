
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
	
	float WeightsSum = 0;
	for (const auto [pBehavior, Weight] : WeightedBehaviors)
	{
		if (Weight == 0.f) continue;
		
		auto const Steering = pBehavior->CalculateSteering(DeltaT, Agent);
		BlendedSteering.LinearVelocity += Steering.LinearVelocity * Weight;
		WeightsSum += Weight;
	}
	
	if (WeightsSum == 0.f)
		return SteeringOutput{};
	
	BlendedSteering.LinearVelocity /= WeightsSum;
	
	// TODO: Calculate the weighted average steeringbehavior
	
	// TODO: Add debug drawing
	DrawDebugLine(
		Agent.GetWorld(),
		Agent.GetTransform().GetLocation(),
		Agent.GetTransform().GetLocation()
			+ FVector{BlendedSteering.LinearVelocity.X, BlendedSteering.LinearVelocity.Y, 0.f},
		FColor::Purple, false, -1, 0, 2.f
	);

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

void BlendedSteering::DebugRender(SteeringOutput const& Output, ASteeringAgent const& Agent) const
{
	for (auto const &Behavior : WeightedBehaviors)
	{
		Behavior.pBehavior_->DebugRender(Output, Agent);
	}
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
