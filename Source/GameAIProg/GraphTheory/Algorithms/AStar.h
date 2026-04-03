#pragma once

#include <vector>
#include "Shared/Graph/Graph.h"
#include "Heuristics.h"

namespace GameAI
{
	class AStar
	{
	public:
		AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction);
		
		std::vector<Node*> FindPath(Node* const pStartNode, Node* const pDestinationNode);

	private:
		float GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const;

		Graph* pGraph;
		HeuristicFunctions::Heuristic HeuristicFunction;
	};
}
