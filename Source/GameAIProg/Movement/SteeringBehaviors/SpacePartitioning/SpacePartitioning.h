/*=============================================================================*/
// SpacePartitioning.h: Contains Cell and Cellspace which are used to partition a space in segments.
// Cells contain pointers to all the agents within.
// These are used to avoid unnecessary distance comparisons to agents that are far away.

// Heavily based on chapter 3 of "Programming Game AI by Example" - Mat Buckland
/*=============================================================================*/

#pragma once
#include <span>
#include <vector>
#include <iterator>
#include <optional>

#include "Debug/ReporterGraph.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"

struct FCell final
{
	std::vector<ASteeringAgent const*> Agents{};
	
	FCell()
	{
		Agents.reserve(5);
	}
	
	void RemoveAgent(ASteeringAgent const *Agent);
};

struct FCellInfo
{
	FCell* pCell;
	FRect Rect;
	size_t CellIndex;
		
	void DebugDraw(UWorld *pWorld) const;
		
	[[nodiscard]]
	bool Contains(FVector2D Pos) const;
};

class FGridPartitioning final
{
	std::vector<FCell> Cells;
	std::vector<FCell*> PreviousAgentCell;
	FRect m_Rect;
	UWorld* m_pWorld;
	int m_CellsPerRow;
	float m_CellSize;

public:
	FGridPartitioning(UWorld* pWorld, float Width, float Height, float CellSize);
	
	[[nodiscard]]
	std::optional<FCellInfo> GetCell(FVector2D Pos);
	
	[[nodiscard]]
	FRect const &GetRect() const;
	
	[[nodiscard]]
	float GetWidth() const
	{
		return m_Rect.Max.X - m_Rect.Min.X;
	}
	
	[[nodiscard]]
	float GetHeight() const
	{
		return m_Rect.Max.Y - m_Rect.Min.Y;
	}
	
	void DebugDraw() const;
	
	void Update(std::span<ASteeringAgent const* const> Agents);
	
	[[nodiscard]]
	bool Contains(FVector2D Pos) const;
	
	void MapNeighborsOf(ASteeringAgent const *Agent, float ScanRadius, std::invocable<ASteeringAgent const*> auto MapFn)
	{
		if (!Contains(Agent->GetPosition())) return;
		
		if (Agent->GetDebugRenderingEnabled())
		{
			DrawDebugBox(m_pWorld, Agent->ToDebugDrawVector(Agent->GetPosition()), FVector{ScanRadius, ScanRadius, 0.f}, FColor::Blue, false, 0);
			Agent->DebugCircleFrom(ScanRadius, FColor::Blue);
		}
		
		FRect const Rect{
			Agent->GetPosition() - ScanRadius,
			Agent->GetPosition() + ScanRadius
		};
		Agent->DebugLineFrom(Rect.Min, FColor::Blue);
		Agent->DebugLineFrom(Rect.Max, FColor::Blue);
		for (auto PosX = Rect.Min.X; PosX <= Rect.Max.X; PosX += m_CellSize)
		{
			for (auto PosY = Rect.Min.Y; PosY <= Rect.Max.Y; PosY += m_CellSize)
			{
				FVector2D const Pos{PosX, PosY};
				
				auto const Cell = GetCell(Pos);
				if (!Cell.has_value()) continue;
				if (Agent->GetDebugRenderingEnabled())
					Cell->DebugDraw(m_pWorld);
				
				for (ASteeringAgent const* NeighborAgent : Cell->pCell->Agents)
				{
					if (NeighborAgent  == Agent) continue;
					MapFn(NeighborAgent);
				}
			}
		}
	}
};