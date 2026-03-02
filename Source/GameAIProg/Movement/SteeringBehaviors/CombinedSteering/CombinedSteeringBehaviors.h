#pragma once
#include <vector>
#include <initializer_list>

#include "../Steering/SteeringBehaviors.h"

//****************
//BLENDED STEERING
class BlendedSteering final: public ISteeringBehavior
{
public:
	struct WeightedBehavior
	{
		ISteeringBehavior* pBehavior_;
		float Weight_;
		
		WeightedBehavior(ISteeringBehavior* const pBehavior, float Weight) :
			pBehavior_{pBehavior},
			Weight_{Weight}
		{
			ensureAlwaysMsgf(Weight >= 0 && Weight <= 1.f, TEXT("Weight must be between 0 and 1, was %f"), Weight);
		}
	};

	explicit BlendedSteering(std::initializer_list<WeightedBehavior> WeightedBehaviors);

	void AddBehaviour(WeightedBehavior WeightedBehavior) { WeightedBehaviors.emplace_back(WeightedBehavior); }
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

	[[nodiscard]]
	float* GetWeight(ISteeringBehavior const* SteeringBehavior);
	
	virtual void DebugRender(SteeringOutput const& Output, ASteeringAgent const& Agent) const override;

private:
	std::vector<WeightedBehavior> WeightedBehaviors;

	// using ISteeringBehavior::SetTarget; // made private because targets need to be set on the individual behaviors, not the combined behavior
};

//*****************
//PRIORITY STEERING
class PrioritySteering final: public ISteeringBehavior
{
public:
	PrioritySteering(const std::vector<ISteeringBehavior*>& priorityBehaviors)
		:m_PriorityBehaviors(priorityBehaviors) 
	{}

	void AddBehaviour(ISteeringBehavior* const pBehavior) { m_PriorityBehaviors.push_back(pBehavior); }
	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

private:
	std::vector<ISteeringBehavior*> m_PriorityBehaviors = {};

	// using ISteeringBehavior::SetTarget; // made private because targets need to be set on the individual behaviors, not the combined behavior
};