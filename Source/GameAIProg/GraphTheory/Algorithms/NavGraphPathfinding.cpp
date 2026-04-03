#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

using namespace GameAI;

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals) 
{
	//Get the start and endTriangle
	FVector2D EffectiveStartPos{};
	auto const StartTri = pNavGraph->GetNavPolygon()->GetClosestTriangleToPosition(startPos, EffectiveStartPos);
	FVector2D EffectiveEndPos{};
	auto const EndTri = pNavGraph->GetNavPolygon()->GetClosestTriangleToPosition(endPos, EffectiveEndPos);
	
	if (!StartTri)
	{
		UE_LOG(LogTemp, Log, TEXT("Start null"), StartTri);
		return {};
	}
	
	if (!EndTri)
	{
		UE_LOG(LogTemp, Log, TEXT("End null"), EndTri);
		return {};
	}
	
	//We have valid start/end triangles and they are not the same
	if (StartTri == EndTri) return {};
	
	//=> Start looking for a path
	//Copy the graph
	NavGraph GraphCopy{*pNavGraph};

	//Create Extra node for the Start Node (Agent's position
	auto const StartNodeId = GraphCopy.AddNode(std::make_unique<Node>(EffectiveStartPos));
	UE_LOG(LogTemp, Log, TEXT("Start ids: %d"), StartTri->MidPointNodeIds.size());
	for (auto const Neighbor : StartTri->MidPointNodeIds)
	{
		GraphCopy.AddConnection(Neighbor, StartNodeId);
	}

	//Create extra node for the endNode
	auto const EndNodeId = GraphCopy.AddNode(std::make_unique<Node>(EffectiveEndPos));
	UE_LOG(LogTemp, Log, TEXT("End ids: %d"), EndTri->MidPointNodeIds.size());
	for (auto const Neighbor : EndTri->MidPointNodeIds)
	{
		GraphCopy.AddConnection(Neighbor, EndNodeId);
	}

	//Run A star on new graph
	AStar AStarAlgo{&GraphCopy, HeuristicFunctions::Chebyshev};

	//Debug Visualisation
	auto const NodePath = AStarAlgo.FindPath(StartNodeId, EndNodeId);
	
	std::vector<FVector2D> FinalPath{};
	FinalPath.reserve(NodePath.size());
	
	std::transform(NodePath.begin(), NodePath.end(), std::back_inserter(FinalPath), [](Node const *pNode)
		{
			return pNode->GetPosition();
		}
	);

	// Extra: Run optimiser on new graph (First check if everything works without SSFA!)
	// debugPortals = SSFA::FindPortals(nodes, *pNavGraph->GetNavPolygon());
	// finalPath = SSFA::OptimizePortals(debugPortals, *pNavGraph->GetNavPolygon());
	
	return FinalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
	std::vector<FVector2D> debugNodePositions{};
	std::vector<NavLine> debugPortals{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}