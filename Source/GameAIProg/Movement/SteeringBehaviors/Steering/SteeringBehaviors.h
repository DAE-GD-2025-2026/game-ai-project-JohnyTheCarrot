#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	[[nodiscard]]
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent &Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

class Arrive : public ISteeringBehavior
{
	float SlowRadius{300.f};
	float TargetRadius{60.f};
	
public:
	Arrive() = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

class Flee : public Seek
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

class Wander : public Seek
{
	float Radius{50.f};
	float Distance{80.f};
	
public:
	using Seek::Seek;
	virtual ~Wander() override = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

class Pursuit : public ISteeringBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

class Evade : public Pursuit
{
	// TODO: ask whether there should be a radius to this.. you can't live a life running for ever
public:
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// Your own SteeringBehaviors should follow here...
