#include "INeighborAnalysis.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"

void FFlockAgentNeighborInfo::ProcessNeighbor(ASteeringAgent const* Neighbor)
{
	auto const AtoBVec = Neighbor->GetPosition() - Neighbor->GetPosition();
	auto const AtoBDist = AtoBVec.Length();
	auto const AtoBNormal = AtoBVec / AtoBDist;
	
	AveragePos += Neighbor->GetPosition();
	CumulativeSeparationVec += -AtoBNormal / AtoBDist; // Normal scaled by inverse of original vector's magnitude
	SeparationWeightTotal += 1.f / AtoBDist;
	AverageVelocity += Neighbor->GetLinearVelocity();
	++NumNeighbors;
}

// void INeighborAnalysis::RecordNeighborRelationship(FFlockAgentNeighborInfo& NeighborA, FFlockAgentNeighborInfo& NeighborB)
// {
// 	auto const AtoBVec = NeighborB.Agent->GetPosition() - NeighborA.Agent->GetPosition();
// 	auto const AtoBDist = AtoBVec.Length();
// 	auto const AtoBNormal = AtoBVec / AtoBDist;
// 	
// 	NeighborA.AveragePos += NeighborB.Agent->GetPosition();
// 	NeighborA.CumulativeSeparationVec += -AtoBNormal / AtoBDist; // Normal scaled by inverse of original vector's magnitude
// 	NeighborA.SeparationWeightTotal += 1.f / AtoBDist;
// 	NeighborA.AverageVelocity += NeighborB.Agent->GetLinearVelocity();
// 	++NeighborA.NumNeighbors;
// 			
// 	NeighborB.AveragePos += NeighborA.Agent->GetPosition();
// 	NeighborB.CumulativeSeparationVec += AtoBNormal / AtoBDist;
// 	NeighborB.SeparationWeightTotal += 1.f / AtoBDist;
// 	NeighborB.AverageVelocity += NeighborA.Agent->GetLinearVelocity();
// 	++NeighborB.NumNeighbors;
// }

void FNaiveNeighborAnalysis::Analyse(
	std::vector<FFlockAgentNeighborInfo>& Neighbors,
	std::span<ASteeringAgent* const> Agents,
	float NeighborhoodRadius
) {
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("Naive Analyse");
	int Iterations = 0;
	
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Reset neighbors");
		for (auto &Neighbor : Neighbors)
			Neighbor = {};
	}
	
	for (auto AgentIt = Agents.begin(); AgentIt != Agents.end(); ++AgentIt)
	{
		if (AgentIt + 1 == Agents.end()) break;
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Outer analysis loop");
		
		auto const AgentIndex = std::distance(Agents.begin(), AgentIt);
		
		for (auto NeighborAgentIt = AgentIt + 1; NeighborAgentIt != Agents.end(); ++NeighborAgentIt)
		{
			auto const NeighborIndex = std::distance(Agents.begin(), NeighborAgentIt);
			
			auto const Distance = (*AgentIt)->GetHorizontalDistanceTo(*NeighborAgentIt);
			if (Distance > NeighborhoodRadius) continue;
			
			TRACE_CPUPROFILER_EVENT_SCOPE_STR("Inner analysis loop");
			Neighbors[AgentIndex].AveragePos += (*NeighborAgentIt)->GetPosition();
			Neighbors[AgentIndex].CumulativeSeparationVec += ((*AgentIt)->GetPosition() - (*NeighborAgentIt)->GetPosition()).GetSafeNormal() / Distance;
			Neighbors[AgentIndex].SeparationWeightTotal += 1.f / Distance;
			Neighbors[AgentIndex].AverageVelocity += (*NeighborAgentIt)->GetLinearVelocity();
			++Neighbors[AgentIndex].NumNeighbors;
			
			Neighbors[NeighborIndex].AveragePos += (*AgentIt)->GetPosition();
			Neighbors[NeighborIndex].CumulativeSeparationVec += ((*NeighborAgentIt)->GetPosition() - (*AgentIt)->GetPosition()).GetSafeNormal() / Distance;
			Neighbors[NeighborIndex].SeparationWeightTotal += 1.f / Distance;
			Neighbors[NeighborIndex].AverageVelocity += (*AgentIt)->GetLinearVelocity();
			++Neighbors[NeighborIndex].NumNeighbors;
			++Iterations;
		}
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("Naive %d"), Iterations);
}

FGridNeighborAnalysis::FGridNeighborAnalysis(UWorld* pWorld, float Width, float Height, float CellSize)
	: GridPartitioning{pWorld, Width, Height, CellSize} {
}

void FGridNeighborAnalysis::DebugDraw() const
{
	if (GetShouldDrawDebug())
		GridPartitioning.DebugDraw();
}

void FGridNeighborAnalysis::Analyse(std::vector<FFlockAgentNeighborInfo>& Neighbors,
                                    std::span<ASteeringAgent* const> Agents, float NeighborhoodRadius)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("Grid Analyse");
	GridPartitioning.Update(Agents, GetShouldDrawDebug());
	
	int Iterations = 0;
	
	for (auto AgentIt = Agents.begin(); AgentIt != Agents.end(); ++AgentIt)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Agent Analyse");
		auto const AgentIndex = std::distance(Agents.begin(), AgentIt);
		
		auto &Neighbor = Neighbors[AgentIndex];
		Neighbor = {};
		
		auto const Cell = GridPartitioning.GetCell((*AgentIt)->GetPosition());
		if (!Cell.has_value()) 
			continue;
		
		GridPartitioning.MapNeighborsOf(*AgentIt, NeighborhoodRadius, [&Iterations, NeighborhoodRadius, AgentIt, &Neighbor](ASteeringAgent const* NeighborAgent)
		{
			auto const DiffVec = (*AgentIt)->GetPosition() - NeighborAgent->GetPosition();
			auto const DistSquared = DiffVec.SquaredLength();
			if (DistSquared > NeighborhoodRadius * NeighborhoodRadius) return;
			
			(*AgentIt)->DebugPoint(NeighborAgent->GetPosition(), FColor::Blue);
			
			auto const InvDistance = FMath::InvSqrt(DistSquared);
			Neighbor.AveragePos += NeighborAgent->GetPosition();
			Neighbor.CumulativeSeparationVec += DiffVec.GetSafeNormal() * InvDistance;
			Neighbor.SeparationWeightTotal += InvDistance;
			Neighbor.AverageVelocity += NeighborAgent->GetLinearVelocity();
			++Neighbor.NumNeighbors;
			++Iterations;
		});
	}
	// UE_LOG(LogTemp, Warning, TEXT("Grid %d"), Iterations);
}
