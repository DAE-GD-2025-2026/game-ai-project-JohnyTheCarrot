#include "NavGraph.h"

#include <unordered_set>

#include "NavGraphNode.h"

GameAI::NavGraph::NavGraph(std::unique_ptr<TriPolygon> && NavPoly)
	: Graph{false}
	, pNavPoly{std::move(NavPoly)}
{
	CreateNavigationGraph();
}

GameAI::NavGraph::NavGraph(const NavGraph& Other)
	: Graph(false)
{
	Nodes.reserve(Other.Nodes.size());
	for (std::unique_ptr<Node> const & OtherNode : Other.Nodes)
	{
		Nodes.push_back(std::make_unique<NavGraphNode>(*static_cast<NavGraphNode*>(OtherNode.get())));
	}
        
	Connections.reserve(Other.Connections.size());
	for (std::unique_ptr<Connection> const & OtherConnection : Other.Connections)
	{
		Connections.push_back(std::make_unique<Connection>(*OtherConnection.get()));
	}
}

std::unique_ptr<GameAI::NavGraph> GameAI::NavGraph::Clone() const
{
	return std::make_unique<NavGraph>(*this);
}

int GameAI::NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
	if (EdgeIdx >= 0)
	{
		for (auto const & pNode : Nodes)
		{
			if (reinterpret_cast<NavGraphNode*>(pNode.get())->GetEdgeIdx() == EdgeIdx)
			{
				return pNode->GetId();
			}
		}
	}
	
	return Graphs::InvalidNodeId;
}

void GameAI::NavGraph::CreateNavigationGraph()
{
	enum class EdgeConnects
	{
		Maybe, // 1 triangle
		Connects // 2 triangles
	};
	
	// Here we store whether edges connect:
	// Edge missing? => haven't seen it (in a triangle) 
	// Edge present => 1st hit we store the first tri, 2nd hit we connect the first with the other tri
	std::unordered_map<TriPolygon::Edge, TriPolygon::Triangle> EdgeConnectionRegistry{};	
	
	// Any edges that **have been found to connect** will be associated with their triangles
	std::unordered_multimap<TriPolygon::Triangle, int> TriNodes{};
	
#pragma region UpdateEdgeConnectionRecord 
	// Utility function for next step
	auto UpdateEdgeConnectionRecord = [this, &TriNodes, &EdgeConnectionRegistry](TriPolygon::Edge const &Edge, TriPolygon::Triangle const &Tri)
	{
		if (auto const It = EdgeConnectionRegistry.find(Edge); It != EdgeConnectionRegistry.end())
		{
			FVector2d const MidPoint{Edge.GetMidPoint(*pNavPoly)};
			int const NodeId = AddNode(std::make_unique<Node>(MidPoint));
    
			auto const FirstTri = It->second;
			TriNodes.emplace(Tri, NodeId);
			TriNodes.emplace(FirstTri, NodeId);
		}
		else
		{
			EdgeConnectionRegistry.emplace(Edge, Tri);
		}
	};
#pragma endregion
	
	// Step 1: we go over all the triangles and take note of any edges we see more than once, as described above
	auto const Triangles = pNavPoly->GetTriangles();
	UE_LOG(LogTemp, Log, TEXT("triangles %d"), Triangles.size());
	for (auto const &Triangle : Triangles)
	{
		auto const [E1, E2, E3] = Triangle.GetEdges();
		UpdateEdgeConnectionRecord(E1, Triangle);
		UpdateEdgeConnectionRecord(E2, Triangle);
		UpdateEdgeConnectionRecord(E3, Triangle);
	}
	
	UE_LOG(LogTemp, Log, TEXT("TriNodes %d"), TriNodes.size());
	
	std::unordered_set<TriPolygon::Triangle> ProcessedTris{};
	// Step 2: We go over all the Triangles with which connecting edges are associated
	for (TriPolygon::Triangle const &Triangle : TriNodes | std::views::keys)
	{
		if (!ProcessedTris.insert(Triangle).second) continue;
		
		auto const [begin, end] = TriNodes.equal_range(Triangle);
		
		// Each node associated with the triangle gets connected with its siblings
		for (auto It = begin; It != end; ++It)
		{
			for (auto [_, OtherNodeId] : std::ranges::subrange(It, end))
			{
				auto const NodeId = It->second;
				
				if (NodeId == OtherNodeId) continue;
				AddConnection(NodeId, OtherNodeId, GetDistanceBetween(NodeId, OtherNodeId));
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("hi %d"), GetNodeCount());
}
