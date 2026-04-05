#pragma once
#include <vector>

namespace GameAI
{
	class NavGraph;

	struct NavLine
	{
		FVector2D P1, P2;	
	};
	
	struct SSFALegsDebug final
	{
		FVector2D Apex;
		std::vector<FVector2D> Left;
		std::vector<FVector2D> Right;
	};

	class NavMeshPathfinding
	{
	public:
		static std::vector<FVector2D> FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph,
			std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals, std::vector<GameAI::SSFALegsDebug> &debugLegs);
		static std::vector<FVector2D> FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph, std::vector<NavLine>& portals, std::vector<GameAI::SSFALegsDebug> &debugLegs);
	};
}
