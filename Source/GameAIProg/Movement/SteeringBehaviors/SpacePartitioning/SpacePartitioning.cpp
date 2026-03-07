#include "SpacePartitioning.h"
//
// // --- Cell ---
// // ------------
// Cell::Cell(float Left, float Bottom, float Width, float Height)
// {
// 	BoundingBox.Min = { Left, Bottom };
// 	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
// }
//
// std::vector<FVector2D> Cell::GetRectPoints() const
// {
// 	const float left = BoundingBox.Min.X;
// 	const float bottom = BoundingBox.Min.Y;
// 	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
// 	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;
//
// 	std::vector<FVector2D> rectPoints =
// 	{
// 		{ left , bottom  },
// 		{ left , bottom + height  },
// 		{ left + width , bottom + height },
// 		{ left + width , bottom  },
// 	};
//
// 	return rectPoints;
// }
//
// // --- Partitioned Space ---
// // -------------------------
// CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
// 	: pWorld{pWorld}
// 	, SpaceWidth{Width}
// 	, SpaceHeight{Height}
// 	, NrOfRows{Rows}
// 	, NrOfCols{Cols}
// 	, NrOfNeighbors{0}
// {
// 	Neighbors.SetNum(MaxEntities);
// 	
// 	//calculate bounds of a cell
// 	CellWidth = Width / Cols;
// 	CellHeight = Height / Rows;
//
// 	// TODO create the cells
// }
//
// void CellSpace::AddAgent(ASteeringAgent& Agent)
// {
// 	// TODO Add the agent to the correct cell
// }
//
// void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
// {
// 	//TODO Check if the agent needs to be moved to another cell.
// 	//TODO Use the calculated index for oldPos and currentPos for this
// }
//
// void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
// {
// 	// TODO Register the neighbors for the provided agent
// 	// TODO Only check the cells that are within the radius of the neighborhood
// }
//
// void CellSpace::EmptyCells()
// {
// 	for (Cell& c : Cells)
// 		c.Agents.clear();
// }
//
// void CellSpace::RenderCells() const
// {
// 	// TODO Render the cells with the number of agents inside of it
// }
//
// int CellSpace::PositionToIndex(FVector2D const & Pos) const
// {
// 	// TODO Calculate the index of the cell based on the position
// 	return 0;
// }
//
// bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
// {
// 	// Check if the rectangles are separated on either axis
// 	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
// 	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
//     
// 	// If they are not separated, they must overlap
// 	return true;
// }

FGridPartitioning::FGridPartitioning(UWorld* pWorld, float Width, float Height, float CellSize)
	: Rect{FVector2D{-Width / 2.f, -Height / 2.f}, FVector2D{Width / 2.f, Height / 2.f}}
	, pWorld{pWorld}
	, CellSize{CellSize}
{
	auto const CellsPerRow = FMath::CeilToInt(Width / CellSize);
	auto const CellsPerCol = FMath::CeilToInt(Height / CellSize);
	
	auto const CellCount = CellsPerRow * CellsPerCol;
	Cells.resize(CellCount);
}

FRect const &FGridPartitioning::GetRect() const
{
	return Rect;
}

void FGridPartitioning::DebugDraw() const
{
	DrawDebugBox(pWorld, FVector::ZeroVector, FVector{GetWidth(), GetHeight(), 0.f}, FColor::Cyan);
}

void Cell::RemoveAgent(ASteeringAgent const* Agent)
{
	auto const AgentIt = std::ranges::find(Agents, Agent);
	if (AgentIt == Agents.end()) return;
	
	// remove through swapping with last + pop
	*AgentIt = Agents.back();
	Agents.pop_back();
}

void FGridPartitioning::FCellInfo::DebugDraw(UWorld *pWorld) const
{
	auto const Width = Rect.Max.X - Rect.Min.X;
	auto const Height = Rect.Max.Y - Rect.Min.Y;
	
	auto const Min3D = FVector{Rect.Min, 0.f};
	auto const Max3D = FVector{Rect.Max, 0.f};
	DrawDebugBox(pWorld, Min3D + (Max3D - Min3D) / 2.f, FVector{Width, Height, 0.f}, FColor::Emerald);
	DrawDebugSolidBox(pWorld, Min3D + (Max3D - Min3D) / 2.f, FVector{Width - 20.f, Height - 20.f, 0.f}, FColor::Purple);
}

bool FGridPartitioning::FCellInfo::Contains(FVector2D Pos) const
{
	return Pos.ComponentwiseAllGreaterOrEqual(Rect.Min)
		&& Pos.ComponentwiseAllLessOrEqual(Rect.Max);
}

std::optional<FGridPartitioning::FCellInfo> FGridPartitioning::GetCell(FVector2D Pos)
{
	if (
		   (Pos.X < Rect.Min.X || Pos.X > Rect.Max.X)
		|| (Pos.Y < Rect.Min.Y || Pos.Y > Rect.Max.Y)
	)
		return std::nullopt;
	
	auto const RelativeX = Pos.X - Rect.Min.X;
	auto const RelativeY = Pos.Y - Rect.Min.Y;
	
	auto const CellX = FMath::FloorToInt(RelativeX / CellSize);
	auto const CellY = FMath::FloorToInt(RelativeY / CellSize);
	
	auto const CellsPerRow = FMath::CeilToInt(GetWidth() / CellSize);
	
	auto const CellIndex = CellY * CellsPerRow + CellX;
	check(unsigned(CellIndex) < Cells.size());
	
	FRect CellRect{};
	CellRect.Min = FVector2D{Rect.Min.X + CellX * CellSize, Rect.Min.Y + CellY * CellSize};
	CellRect.Max = FVector2D{Rect.Min.X + CellX * CellSize + CellSize, Rect.Min.Y + CellY * CellSize + CellSize};
	
	return FCellInfo{
		&Cells[CellIndex],
		CellRect
	};
}

void FGridPartitioning::Update(std::span<ASteeringAgent const* const> Agents)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("GridPartitioning Update");
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Clear cells");
		for (auto &[CellAgents] : Cells)
		{
			CellAgents.clear();
		}
	}
	
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Fill cells");
		for (ASteeringAgent const* Agent : Agents)
		{
			auto const Pos = Agent->GetPosition();
			if (auto const Cell = this->GetCell(Pos); Cell.has_value())
			{
				// Cell->DebugDraw(pWorld);
				Cell->pCell->Agents.emplace_back(Agent);
			}
		}
	}
}
