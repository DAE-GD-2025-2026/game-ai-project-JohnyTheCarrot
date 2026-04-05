#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

using namespace GameAI;

std::vector<FVector2d> SSFAlgo(
    std::vector<Node*> const &UnoptimizedPath,
    int StartNodeId,
    int EndNodeId,
    int ClosestMidToStartId,
    NavGraph const &navGraph,
    std::vector<NavLine>& debugPortals,
    std::vector<SSFALegsDebug> &debugLegs
)
{
    if (UnoptimizedPath.empty()) return {};
    
    debugPortals.clear();
    
	for (auto const *Node : UnoptimizedPath)
	{
		auto const Id = Node->GetId();
		if (Id == StartNodeId || Id == EndNodeId || Id == ClosestMidToStartId) continue;

		auto const Edge = navGraph.GetEdgeFromNodeId(Id).value();
		FVector2d P1{Edge.first};
		FVector2d P2{Edge.second};
		debugPortals.emplace_back(P1, P2);
	}

	for (int i = 1; i < static_cast<int>(debugPortals.size()); ++i)
	{
		if (debugPortals[i].P1 == debugPortals[i-1].P2 || 
			debugPortals[i].P2 == debugPortals[i-1].P1)
			std::swap(debugPortals[i].P1, debugPortals[i].P2);
	}	
	
    if (debugPortals.empty())
        return { navGraph.GetNode(StartNodeId)->GetPosition(),
                 navGraph.GetNode(EndNodeId)->GetPosition() };

    std::vector<FVector2d> Path{};
	FVector2d Apex{navGraph.GetNode(ClosestMidToStartId)->GetPosition()}; 
    FVector2d LeftLeg{debugPortals[0].P1};
    FVector2d RightLeg{debugPortals[0].P2};
    int leftLegIndex = 0;
    int rightLegIndex = 0;
    Path.emplace_back(Apex);
	
	for (int i = 0; i < debugPortals.size(); ++i)
	{
		UE_LOG(LogTemp, Warning, TEXT("Portal %d: P1=(%.1f, %.1f) P2=(%.1f, %.1f)"),
			i,
			debugPortals[i].P1.X, debugPortals[i].P1.Y,
			debugPortals[i].P2.X, debugPortals[i].P2.Y);
	}
	UE_LOG(LogTemp, Warning, TEXT("Apex: (%.1f, %.1f)"), Apex.X, Apex.Y);
	UE_LOG(LogTemp, Warning, TEXT("End: (%.1f, %.1f)"),
		navGraph.GetNode(EndNodeId)->GetPosition().X,
		navGraph.GetNode(EndNodeId)->GetPosition().Y);

    auto const Cross = [](FVector2d Origin, FVector2d A, FVector2d B)
    {
        return FVector2d::CrossProduct(A - Origin, B - Origin);
    };
    
    for (int PortalIndex = 1; PortalIndex < static_cast<int>(debugPortals.size()); ++PortalIndex)
    {
        auto const &[PortalP1, PortalP2] = debugPortals[PortalIndex];
        
        FVector2d const NewRightLeg = PortalP1;
        if (Cross(Apex, RightLeg, NewRightLeg) <= 0)
        {
            if (Cross(Apex, LeftLeg, NewRightLeg) < 0)
            {
                Apex = LeftLeg;
                Path.emplace_back(Apex);
                PortalIndex = leftLegIndex + 1;
            	if (PortalIndex >= static_cast<int>(debugPortals.size()))
            		break;
                rightLegIndex = PortalIndex;
                leftLegIndex = PortalIndex;
                LeftLeg  = debugPortals[PortalIndex].P1;
                RightLeg = debugPortals[PortalIndex].P2;
            	--PortalIndex;
                continue;
            }
            RightLeg = NewRightLeg;
            rightLegIndex = PortalIndex;
        }
        
        FVector2d const NewLeftLeg = PortalP2;
        if (Cross(Apex, LeftLeg, NewLeftLeg) >= 0)
        {
            if (Cross(Apex, RightLeg, NewLeftLeg) > 0)
            {
                Apex = RightLeg;
                Path.emplace_back(Apex);
                PortalIndex = rightLegIndex + 1;
            	if (PortalIndex >= static_cast<int>(debugPortals.size()))
            		break;
                leftLegIndex = PortalIndex;
                rightLegIndex = PortalIndex;
                LeftLeg  = debugPortals[PortalIndex].P1;
                RightLeg = debugPortals[PortalIndex].P2;
            	--PortalIndex;
                continue;
            }
            LeftLeg = NewLeftLeg;
            leftLegIndex = PortalIndex;
        }
    }
    
    Path.emplace_back(navGraph.GetNode(EndNodeId)->GetPosition());
    
    return Path;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals, std::vector<GameAI::SSFALegsDebug> &debugLegs) 
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
	if (StartTri == EndTri) return {EffectiveStartPos, EffectiveEndPos};
	
	//=> Start looking for a path
	//Copy the graph
	NavGraph GraphCopy{*pNavGraph};

	//Create Extra node for the Start Node (Agent's position
	auto const StartNodeId = GraphCopy.AddNode(std::make_unique<Node>(EffectiveStartPos));
	UE_LOG(LogTemp, Log, TEXT("Start ids: %d"), StartTri->MidPointNodeIds.size());
	if (StartTri->MidPointNodeIds.empty())
	{
		UE_LOG(LogTemp, Log, TEXT("No start ids"));
		return {};
	}
	auto const ClosestMidToStart = *std::min_element(StartTri->MidPointNodeIds.begin(), StartTri->MidPointNodeIds.end(), [&GraphCopy, StartNodeId](int NodeAId, int NodeBId)
	{
		return GraphCopy.GetDistanceBetween(StartNodeId, NodeAId) < GraphCopy.GetDistanceBetween(StartNodeId, NodeBId);
	});
	GraphCopy.AddConnection(ClosestMidToStart, StartNodeId);

	//Create extra node for the endNode
	auto const EndNodeId = GraphCopy.AddNode(std::make_unique<Node>(EffectiveEndPos));
	UE_LOG(LogTemp, Log, TEXT("End ids: %d"), EndTri->MidPointNodeIds.size());
	if (EndTri->MidPointNodeIds.empty())
	{
		UE_LOG(LogTemp, Log, TEXT("No end ids"));
		return {};
	}
	auto const ClosestMidToEnd = *std::min_element(EndTri->MidPointNodeIds.begin(), EndTri->MidPointNodeIds.end(), [&GraphCopy, EndNodeId](int NodeAId, int NodeBId)
	{
		return GraphCopy.GetDistanceBetween(EndNodeId, NodeAId) < GraphCopy.GetDistanceBetween(EndNodeId, NodeBId);
	});
	GraphCopy.AddConnection(ClosestMidToEnd, EndNodeId);

	//Run A star on new graph
	AStar AStarAlgo{&GraphCopy, HeuristicFunctions::Chebyshev};

	//Debug Visualisation
	auto const NodePath = AStarAlgo.FindPath(StartNodeId, EndNodeId);
	check(NodePath.size());
	
	std::vector<FVector2D> FinalPath{};
	FinalPath.reserve(NodePath.size());
	
	// std::transform(NodePath.begin(), NodePath.end(), std::back_inserter(FinalPath), [](Node const *pNode)
	// 	{
	// 		return pNode->GetPosition();
	// 	}
	// );

	// Extra: Run optimiser on new graph (First check if everything works without SSFA!)
	// debugPortals = SSFA::FindPortals(NodePath, *pNavGraph, StartPortal, EndPortal);
	FinalPath = SSFAlgo(NodePath, StartNodeId, EndNodeId, ClosestMidToStart, GraphCopy, debugPortals, debugLegs);
	// FinalPath = SSFA::OptimizePortals(debugPortals, *pNavGraph->GetNavPolygon());
	
	UE_LOG(LogTemp, Warning, TEXT("Nav poly edges"));
	for (auto const &El : pNavGraph->GetNavPolygon()->GetEdges())
	{
		auto const P1_X = El.GetP1(*pNavGraph->GetNavPolygon()).X;
		auto const P1_Y = El.GetP1(*pNavGraph->GetNavPolygon()).Y;
		auto const P2_X = El.GetP2(*pNavGraph->GetNavPolygon()).X;
		auto const P2_Y = El.GetP2(*pNavGraph->GetNavPolygon()).Y;
		UE_LOG(LogTemp, Warning, TEXT("Edge: (%.1f, %.1f) (%.1f, %.1f)"), P1_X, P1_Y, P2_X, P2_Y);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Portals"));
	for (auto const &[P1, P2] : debugPortals)
	{
		auto const P1_X = P1.X;
		auto const P1_Y = P1.Y;
		auto const P2_X = P2.X;
		auto const P2_Y = P2.Y;
		UE_LOG(LogTemp, Warning, TEXT("Portal: (%.1f, %.1f) (%.1f, %.1f)"), P1_X, P1_Y, P2_X, P2_Y);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Unoptimized path"));
	for (auto const *Node : NodePath)
	{
		auto const P1 = Node->GetPosition();
		auto const P1_X = P1.X;
		auto const P1_Y = P1.Y;
		UE_LOG(LogTemp, Warning, TEXT("Point: (%.1f, %.1f)"), P1_X, P1_Y);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Path"));
	for (auto const &P : FinalPath)
	{
		UE_LOG(LogTemp, Warning, TEXT("Path: (%.1f, %.1f)"), P.X, P.Y);
	}
	
	return FinalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph, std::vector<NavLine>& portals, std::vector<GameAI::SSFALegsDebug> &debugLegs)
{
	std::vector<FVector2D> debugNodePositions{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, portals, debugLegs);
}