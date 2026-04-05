#pragma once

#include <vector>
#include <unordered_map>
#include "Shared/Graph/Graph.h"
#include "Heuristics.h"

namespace GameAI
{
	class AStar
	{
	public:
		AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction);
		
		[[nodiscard]]
		std::vector<Node*> FindPath(Node* const pStartNode, Node* const pDestinationNode);
		[[nodiscard]]
		std::vector<Node*> FindPath(int StartNodeId, int EndNodeId)
		{
			return FindPath(pGraph->GetNode(StartNodeId), pGraph->GetNode(EndNodeId));
		}

	private:
		float GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const;

		Graph* pGraph;
		HeuristicFunctions::Heuristic HeuristicFunction;
	};
}
