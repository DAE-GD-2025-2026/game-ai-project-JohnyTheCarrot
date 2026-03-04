#pragma once
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
class Flock;

class FlockBehavior
{
	Flock const *pFlock_;
	
protected:
	[[nodiscard]]
	Flock const &GetFlock() const noexcept { return *pFlock_; }
	
public:
	explicit FlockBehavior(Flock const &Flock)
		: pFlock_{&Flock}
		{}
	
	virtual ~FlockBehavior() = default;
};

//COHESION - FLOCKING
//*******************
class Cohesion final : public Seek, protected FlockBehavior
{
public:
	using FlockBehavior::FlockBehavior;

	//Cohesion Behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

//SEPARATION - FLOCKING
//*********************
class Separation final : public Flee, protected FlockBehavior
{
public:
	using FlockBehavior::FlockBehavior;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

//VELOCITY MATCH - FLOCKING
//************************
class Alignment final : public Flee, protected FlockBehavior
{
public:
	explicit Alignment(Flock const &Flock)
		: Flee{}
		, FlockBehavior{Flock}
		{}
	
	virtual ~Alignment() override = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};
