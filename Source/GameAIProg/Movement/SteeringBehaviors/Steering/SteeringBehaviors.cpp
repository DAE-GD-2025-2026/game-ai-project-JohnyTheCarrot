#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

void ISteeringBehavior::SetTarget(ASteeringAgent const* TargetAgent)
{
	check(TargetAgent != nullptr);
	
	FTargetData Target;
	Target.Position = TargetAgent->GetPosition();
	Target.Orientation = TargetAgent->GetRotation();
	Target.LinearVelocity = TargetAgent->GetLinearVelocity();
	Target.AngularVelocity = TargetAgent->GetAngularVelocity();
	
	SetTarget(Target);
}

//SEEK
//*******
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	FVector2D const ToTargetVector{Target.Position.X - Agent.GetActorLocation().X, Target.Position.Y - Agent.GetActorLocation().Y};
	
	SteeringOutput Result{};
	Result.LinearVelocity = ToTargetVector.GetSafeNormal();
	
	Agent.DebugLineRelative(Result.LinearVelocity, FColor::Yellow);
	
	return Result;
}

SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	FVector2D ToTargetVector{Target.Position.X - Agent.GetActorLocation().X, Target.Position.Y - Agent.GetActorLocation().Y};
	auto const DistanceSquared = ToTargetVector.SquaredLength();
	
	FColor TargetRadiusCircleColor = FColor::Red;
	FColor SlowRadiusCircleColor = FColor::White;
	
	SteeringOutput Result{};
	float SpeedFactor = 1.f;
	if (auto const SlowRadiusSquared = SlowRadius * SlowRadius; DistanceSquared <= SlowRadiusSquared)
	{
		SpeedFactor = DistanceSquared / SlowRadiusSquared;
		SlowRadiusCircleColor = FColor::Cyan;
		
		if (DistanceSquared <= TargetRadius * TargetRadius)
		{
			Result.IsValid = false;
			TargetRadiusCircleColor = FColor::Green;
			ToTargetVector = FVector2D::ZeroVector;
			SpeedFactor = 1.f;
		}
	}
	
	Agent.SetMaxLinearSpeed(SpeedFactor * Agent.GetOriginalMaxLinearSpeed());
	Result.LinearVelocity = ToTargetVector;
	
	Agent.DebugCircle(Target.Position, TargetRadius, TargetRadiusCircleColor);
	Agent.DebugCircle(Target.Position, SlowRadius, SlowRadiusCircleColor);
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

SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	auto const AgentToTargetVector = Target.Position - Agent.GetPosition();
	auto const TimeToReachTarget = AgentToTargetVector.Length() / Agent.GetMaxLinearSpeed();
	
	auto const PredictedTarget = Target.Position + TimeToReachTarget * Target.LinearVelocity;
	
	Agent.DebugLine(Target.Position, PredictedTarget, FColor::Cyan);
	Agent.DebugLineFrom(PredictedTarget, FColor::Magenta);
	
	SteeringOutput Result{};
	Result.LinearVelocity = PredictedTarget - Agent.GetPosition();
	
	return Result;
}

SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	auto const TargetDistSquared = FVector2D::DistSquared(Target.Position, Agent.GetPosition());
	auto const RadiusSquared = Radius * Radius;
	
	Agent.DebugCircle(Target.Position, Radius, FColor::Green);
	Agent.DebugText(Target.Position, FColor::Green, TEXT("Oh BROTHER this guy STINKS!"));
	
	if (TargetDistSquared >= RadiusSquared)
	{
		SteeringOutput Result{};
		Result.IsValid = false;
		
		return Result;
	}
	
	auto PursuitResult = Pursuit::CalculateSteering(DeltaT, Agent);
	
	PursuitResult.LinearVelocity = -PursuitResult.LinearVelocity;
	
	return PursuitResult;
}
