#include "AStar.h"

#include <queue>
#include <deque>
#include <unordered_set>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

// stores the optimal connection to a node and its total costs related to the start and end node of the path
struct NodeRecord final
{
	Node* pNode = nullptr;
	// Connection* pConnection = nullptr;
	float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
	float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

	bool operator==(const NodeRecord& other) const
	{
		return pNode == other.pNode
			// && pConnection == other.pConnection
			&& costSoFar == other.costSoFar
			&& estimatedTotalCost == other.estimatedTotalCost;
	};

	bool operator<(const NodeRecord& other) const
	{
		return estimatedTotalCost < other.estimatedTotalCost;
	};
};

std::vector<Node*> AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::vector<NodeRecord> openSet{};
	std::unordered_set<Node*> closedSet;
	
	openSet.emplace_back(
		NodeRecord{
			.pNode = pStartNode,
			.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode),
		}
	);
	std::unordered_map<Node*, Node*> CameFrom;
	
	while (!openSet.empty())
	{
		std::sort(openSet.begin(), openSet.end());
		auto const CurrentRecord = openSet.front();
		
		if (CurrentRecord.pNode == pGoalNode)
		{
			std::vector<Node*> Path;
			Path.reserve(CameFrom.size());
			
			auto CurrentNode = CurrentRecord.pNode;
			while (CameFrom.contains(CurrentNode))
			{
				CurrentNode = CameFrom[CurrentNode];
				Path.emplace_back(CurrentNode);
			}
			
			std::ranges::reverse(Path);
			
			return Path;
		}
		
		openSet.erase(openSet.begin());
		closedSet.emplace(CurrentRecord.pNode);
		
		auto const Connections = pGraph->FindConnectionsFrom(CurrentRecord.pNode->GetId());
		
		for (auto const *Connection : Connections)
		{
			auto const NeighborNode = pGraph->GetNode(Connection->GetToId());
			if (closedSet.contains(NeighborNode)) continue;
			auto &NeighborRecord = [this, pGoalNode, &openSet, NeighborNode]() -> NodeRecord &
			{
				auto const NeighborIt = std::ranges::find_if(openSet, [NeighborNode](auto const &Record) { return Record.pNode == NeighborNode;});
				if (NeighborIt == openSet.end())
				{
					return openSet.emplace_back(NodeRecord{
						.pNode = NeighborNode,
						.costSoFar = std::numeric_limits<float>::max(),
						.estimatedTotalCost = GetHeuristicCost(NeighborNode, pGoalNode),
					});
				}
				
				return *NeighborIt;
			}();
			
			auto const TentativeGScore = CurrentRecord.costSoFar + Connection->GetWeight();
			if (TentativeGScore >= NeighborRecord.costSoFar)
				continue;
			
			CameFrom[NeighborNode] = CurrentRecord.pNode;
			NeighborRecord.costSoFar = TentativeGScore;
			NeighborRecord.estimatedTotalCost = TentativeGScore + GetHeuristicCost(NeighborNode, pGoalNode);
		}
	}
	
	checkNoEntry();
	return {};
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}