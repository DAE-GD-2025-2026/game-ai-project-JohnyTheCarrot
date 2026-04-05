#pragma once
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include <unordered_map>

namespace GameAI
{
	class NavGraph : public Graph
	{
	public:
		explicit NavGraph(std::unique_ptr<TriPolygon> && NavPoly);
		NavGraph(const NavGraph& Other);
		
		std::unique_ptr<NavGraph> Clone() const;
		
		TriPolygon const * GetNavPolygon() const {return pNavPoly.get();}
		int GetNodeIdFromEdgeIndex(int EdgeIdx) const;
		
		[[nodiscard]]
		std::optional<std::pair<FVector, FVector>> GetEdgeFromNodeId(int NodeId) const;
		
	private:
		std::unique_ptr<TriPolygon> pNavPoly;
		std::unordered_map<int, TriPolygon::Edge> NodeIdToEdgeIndex;

		void CreateNavigationGraph();
	};
}
