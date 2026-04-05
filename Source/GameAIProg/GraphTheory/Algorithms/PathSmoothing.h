#pragma once
#include <vector>

#include "AudioMixerBlueprintLibrary.h"
#include "NavGraphPathfinding.h"
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

namespace GameAI
{
	class SSFA final
{
public:
	//=== SSFA Functions ===
	//--- References ---
	//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
	//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
	static std::vector<NavLine> FindPortals(std::vector<Node*> const & Path, NavGraph const & NavGraph, std::pair<FVector, FVector> StartPortal, std::pair<FVector, FVector> EndPortal)
	{
		//Container
		std::vector<NavLine> Portals{};
		Portals.reserve(Path.size());
		
		Portals.emplace_back(FVector2d{StartPortal.first}, FVector2d{StartPortal.second});
		if (Path.size() >= 3)
		{
			for (auto const *Node : std::ranges::subrange(Path.begin() + 1, Path.end() - 1))
			{
				auto const [EdgeStart, EdgeEnd] = NavGraph.GetEdgeFromNodeId(Node->GetId()).value();
				Portals.emplace_back(FVector2d{EdgeStart}, FVector2d{EdgeEnd});
			}
		}
		Portals.emplace_back(FVector2d{EndPortal.first}, FVector2d{EndPortal.second});
		
		//For each node received, get it's corresponding line
		
			//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point

			//Store portal

		//Add degenerate portal to force end evaluation

		return Portals;
	}

	static std::vector<FVector2D> OptimizePortals( std::vector<NavLine> const & Portals, NavGraph const &NavGraph)
	{
		std::vector<FVector2D> Path{};
		
		FVector2d Apex{Path[0]};
		bool LastCross = false;
		auto const IsP1LeftOfApex = FVector2d::CrossProduct(Portals[0].P1 - Apex, Portals[0].P2 - Apex) < 0;
		auto [LeftLeg, RightLeg] = IsP1LeftOfApex
			? std::pair(Portals[0].P2, Portals[0].P1) : std::pair(Portals[0].P1, Portals[0].P2);
		
		for (auto const &[P1, P2] : Portals)
		{
			auto const CrossResult = FVector2d::CrossProduct(LeftLeg - Apex, P2 - Apex) < 0;
			if (LastCross != IsP1LeftOfApex)
			{
				Apex = LeftLeg;
				Path.emplace_back(Apex);
			}
			LastCross = IsP1LeftOfApex;
		}
		// auto const [EdgeStart, EdgeEnd] = NavGraph.GetEdgeFromNodeId(Node->GetId()).value();
		// auto const NodePos = Node->GetPosition();
		// auto const IsEdgeStartOnLeftSide = FVector::CrossProduct(EdgeStart - FVector{NodePos, EdgeStart.Z}, EdgeEnd - FVector{NodePos, EdgeEnd.Z}).Z < 0;
		// auto const [Left, Right] = IsEdgeStartOnLeftSide
		// 	? std::pair(EdgeEnd, EdgeStart) : std::pair(EdgeStart, EdgeEnd);
		// Portals.emplace_back(FVector2d{Left}, FVector2d{Right});
		
		//P1 == right point of portal, P2 == left point of portal
		
			//--- RIGHT CHECK ---
			//1. See if moving funnel inwards - RIGHT
			
				//2. See if new line degenerates a line segment - RIGHT
				
					//Leftleg becomes new apex point

					//Calculate new legs (if not the end)


			//--- LEFT CHECK ---
			//1. See if moving funnel inwards - LEFT

				//2. See if new line degenerates a line segment - LEFT

					//Rightleg becomes new apex point

					//Calculate new legs (if not the end)


		// Add last path point

		return Path;
	}
private:
	SSFA() {};
	~SSFA() {};
};
}
