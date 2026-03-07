#pragma once

#include <vector>
#include <span>

#include "Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"

class FGridPartitioning;
class ASteeringAgent;

struct FFlockAgentNeighborInfo final
{
	ASteeringAgent* Agent{};
	FVector2D AveragePos{};
	int NumNeighbors{};
	FVector2D CumulativeSeparationVec{};
	float SeparationWeightTotal{};
	FVector2D AverageVelocity{};
		
	void ProcessNeighbor(ASteeringAgent const *Neighbor);
};

class INeighborAnalysis
{
public:
	// We are switching to this implementation, we want to make sure its state is accurate
	virtual void Awaken(std::span<ASteeringAgent* const> Agents) {}
	
	virtual void Analyse(std::vector<FFlockAgentNeighborInfo> &Neighbors, std::span<ASteeringAgent *const> Agents, float NeighborhoodRadius) = 0;
	
	virtual void DebugDraw() const {}
	
	virtual ~INeighborAnalysis() = default;
};

class FNaiveNeighborAnalysis final : public INeighborAnalysis
{
public:
	using INeighborAnalysis::INeighborAnalysis;
	
	virtual void Analyse(std::vector<FFlockAgentNeighborInfo>& Neighbors, std::span<ASteeringAgent* const> Agents, float NeighborhoodRadius) override;
};

class FGridNeighborAnalysis final : public INeighborAnalysis
{
	FGridPartitioning GridPartitioning;
	
public:
	FGridNeighborAnalysis(UWorld* pWorld, float Width, float Height, float CellSize);
	
	virtual void DebugDraw() const override;
	
	virtual void Awaken(std::span<ASteeringAgent* const> Agents) override;
	
	virtual void Analyse(std::vector<FFlockAgentNeighborInfo>& Neighbors, std::span<ASteeringAgent* const> Agents, float NeighborhoodRadius) override;;
};