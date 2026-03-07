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

// --- Cell ---
// ------------
// struct Cell final
// {
// 	Cell(float Left, float Bottom, float Width, float Height);
//
// 	std::vector<FVector2D> GetRectPoints() const;
// 	
// 	// all the agents currently in this cell
// 	std::list<ASteeringAgent*> Agents;
// 	FRect BoundingBox;
// };
//
// // --- Partitioned Space ---
// // -------------------------
// class CellSpace final
// {
// public:
// 	CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities);
//
// 	void AddAgent(ASteeringAgent& Agent);
// 	void UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos);
//
// 	void RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius);
// 	const TArray<ASteeringAgent*>& GetNeighbors() const { return Neighbors; }
// 	int GetNrOfNeighbors() const { return NrOfNeighbors; }
//
// 	//empties the cells of entities
// 	void EmptyCells();
// 	void RenderCells()const;
//
// private:
// 	// For debug draw purposes
// 	UWorld* pWorld{};
// 	
// 	// Cells and properties
// 	std::vector<Cell> Cells;
// 	FVector2D CellOrigin{};
// 	
// 	float SpaceWidth;
// 	float SpaceHeight;
//
// 	int NrOfRows;
// 	int NrOfCols;
//
// 	float CellWidth;
// 	float CellHeight;
//
// 	// Members to avoid memory allocation on every frame
// 	TArray<ASteeringAgent*> Neighbors;
// 	int NrOfNeighbors;
//
// 	// Helper functions
// 	int PositionToIndex(FVector2D const & Pos) const;
// 	bool DoRectsOverlap(FRect const& RectA, FRect const& RectB);
// };

struct Cell final
{
	std::vector<ASteeringAgent const*> Agents{};
	
	Cell()
	{
		Agents.reserve(5);
	}
	
	void RemoveAgent(ASteeringAgent const *Agent);
};

class FGridPartitioning final
{
	std::vector<Cell> Cells;
	std::vector<Cell*> PreviousAgentCell;
	FRect Rect;
	UWorld* pWorld;
	float CellSize;
	
	struct FCellInfo
	{
		Cell* pCell;
		FRect Rect;
		
		void DebugDraw(UWorld *pWorld) const;
		
		[[nodiscard]]
		bool Contains(FVector2D Pos) const;
	};
	
public:
	FGridPartitioning(UWorld* pWorld, float Width, float Height, float CellSize);
	
	[[nodiscard]]
	std::optional<FCellInfo> GetCell(FVector2D Pos);
	
	[[nodiscard]]
	FRect const &GetRect() const;
	
	[[nodiscard]]
	float GetWidth() const
	{
		return Rect.Max.X - Rect.Min.X;
	}
	
	[[nodiscard]]
	float GetHeight() const
	{
		return Rect.Max.Y - Rect.Min.Y;
	}
	
	void DebugDraw() const;
	
	void Update(std::span<ASteeringAgent const* const> Agents);
	
	void MapNeighborsOf(ASteeringAgent const *Agent, std::invocable<ASteeringAgent const*> auto MapFn)
	{
		auto const Cell = this->GetCell(Agent->GetPosition());
		if (!Cell.has_value()) return;
		
		for (ASteeringAgent const* SteeringAgent : Cell->pCell->Agents)
		{
			if (SteeringAgent == Agent) continue;
			MapFn(SteeringAgent);
		}
	}
};