#include "SpacePartitioning.h"

FGridPartitioning::FGridPartitioning(UWorld* pWorld, float Width, float Height, float CellSize)
	: m_Rect{FVector2D{-Width / 2.f, -Height / 2.f}, FVector2D{Width / 2.f, Height / 2.f}}
	, m_pWorld{pWorld}
	, m_CellsPerRow{FMath::CeilToInt(Width / CellSize)}
	, m_CellSize{CellSize}
{
	auto const CellsPerCol = FMath::CeilToInt(Height / CellSize);
	
	auto const CellCount = m_CellsPerRow * CellsPerCol;
	Cells.resize(CellCount);
}

FRect const &FGridPartitioning::GetRect() const
{
	return m_Rect;
}

void FGridPartitioning::DebugDraw() const
{
	DrawDebugBox(m_pWorld, FVector::ZeroVector, FVector{GetWidth(), GetHeight(), 0.f}, FColor::Cyan);
	
	for (float X = m_Rect.Min.X; X < m_Rect.Max.X; X += m_CellSize)
	{
		DrawDebugLine(
			m_pWorld,
			FVector{X, m_Rect.Min.Y, 0.f},
			FVector{X, m_Rect.Max.Y, 0.f},
			FColor::Green
		);
	}
		
	for (float Y = m_Rect.Min.Y; Y < m_Rect.Max.Y; Y += m_CellSize)
	{
		DrawDebugLine(
			m_pWorld,
			FVector{m_Rect.Min.X, Y, 0.f},
			FVector{m_Rect.Max.X, Y, 0.f},
			FColor::Green
		);
	}
}

void FCell::RemoveAgent(ASteeringAgent const* Agent)
{
	auto const AgentIt = std::ranges::find(Agents, Agent);
	if (AgentIt == Agents.end()) return;
	
	// remove through swapping with last + pop
	*AgentIt = Agents.back();
	Agents.pop_back();
}

void FCellInfo::DebugDraw(UWorld *pWorld) const
{
	auto const Width = Rect.Max.X - Rect.Min.X;
	auto const Height = Rect.Max.Y - Rect.Min.Y;
	
	auto const Min3D = FVector{Rect.Min, 0.f};
	auto const Max3D = FVector{Rect.Max, 0.f};
	DrawDebugPoint(pWorld, Min3D + (Max3D - Min3D) / 2.f, 10.f, FColor::Purple);
	DrawDebugBox(pWorld, Min3D + (Max3D - Min3D) / 2.f, FVector{Width / 2.f, Height / 2.f, 0.f}, FColor::Emerald, false, 0);
	DrawDebugSolidBox(pWorld, Min3D + (Max3D - Min3D) / 2.f, FVector{Width / 2.f - 20.f, Height / 2.f - 20.f, 0.f}, FColor::Purple, false, 0);
}

bool FCellInfo::Contains(FVector2D Pos) const
{
	return Pos.ComponentwiseAllGreaterOrEqual(Rect.Min)
		&& Pos.ComponentwiseAllLessOrEqual(Rect.Max);
}

std::optional<FCellInfo> FGridPartitioning::GetCell(FVector2D Pos)
{
	if (!Contains(Pos))
		return std::nullopt;
	
	auto const RelativeX = Pos.X - m_Rect.Min.X;
	auto const RelativeY = Pos.Y - m_Rect.Min.Y;
	
	auto const CellX = FMath::FloorToInt(RelativeX / m_CellSize);
	auto const CellY = FMath::FloorToInt(RelativeY / m_CellSize);
	
	auto const CellsPerRow = FMath::CeilToInt(GetWidth() / m_CellSize);
	
	auto const CellIndex = CellY * CellsPerRow + CellX;
	check(unsigned(CellIndex) < Cells.size());
	
	FRect CellRect{};
	CellRect.Min = FVector2D{m_Rect.Min.X + CellX * m_CellSize, m_Rect.Min.Y + CellY * m_CellSize};
	CellRect.Max = FVector2D{m_Rect.Min.X + CellX * m_CellSize + m_CellSize, m_Rect.Min.Y + CellY * m_CellSize + m_CellSize};
	
	return FCellInfo{
		&Cells[CellIndex],
		CellRect,
		size_t(CellIndex)
	};
}

void FGridPartitioning::Update(std::span<ASteeringAgent const* const> Agents, bool DrawDebug)
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
				Cell->pCell->Agents.emplace_back(Agent);
				if (DrawDebug)
					DrawDebugLine(
						m_pWorld,
						Agent->ToDebugDrawVector(Agent->GetPosition()),
						Agent->ToDebugDrawVector(Cell->Rect.Min + (Cell->Rect.Max - Cell->Rect.Min) / 2.),
						FColor::Purple
					);
			}
		}
	}
}

bool FGridPartitioning::Contains(FVector2D Pos) const
{
	return
		   (Pos.X >= m_Rect.Min.X && Pos.X < m_Rect.Max.X)
		&& (Pos.Y >= m_Rect.Min.Y && Pos.Y < m_Rect.Max.Y);
}
