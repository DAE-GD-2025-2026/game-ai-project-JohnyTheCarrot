#pragma once
#include <stack>
#include <unordered_set>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<Node*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::unordered_set<int>& visited, Node *pStartNode) const;
		bool GetIsConnected() const;
		
		[[nodiscard]]
		std::optional<std::pair<Node*, Node*>> GetOddPair() const
		{
			std::pair<Node*, Node*> oddPair;
			
			for (auto const pNode : m_pGraph->GetActiveNodes())
			{
				auto const Degree = m_pGraph->GetDegree(pNode->GetId());
				
				if (Degree % 2 == 0) continue;
				if (oddPair.first == nullptr) oddPair.first = pNode;
				else if (oddPair.second == nullptr) oddPair.second = pNode;
				else break;
			}
			
			return oddPair;
		}

		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{
		// TODO If the graph is not connected, there can be no Eulerian Trail
		auto const NumNodes = m_pGraph->GetNodeCount();
		
		auto const IsConnected{GetIsConnected()};
		if (!IsConnected) return Eulerianity::notEulerian;

		// TODO Count nodes with odd degree 
		auto const NumOddDegrees = [this]
		{
			int NumOdd = 0;
			
			for (auto const pNode : m_pGraph->GetActiveNodes())
			{
				auto const Degree = m_pGraph->GetDegree(pNode->GetId());
				if (Degree % 2 != 0) ++NumOdd;
			}
			
			return NumOdd;
		}();

		// TODO A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (NumOddDegrees > 2) return Eulerianity::notEulerian;
		
		// TODO A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (unless there are only 2 nodes)
		if (NumOddDegrees == 2 && NumNodes != 2)
			return Eulerianity::semiEulerian;
			
		// TODO An Euler trail can be made, but only starting and ending in these 2 nodes

		// TODO A connected graph with no odd nodes is Eulerian
		if (NumOddDegrees == 0) return Eulerianity::eulerian;
		
		return Eulerianity::notEulerian;
	}

	inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		Graph graphCopy = m_pGraph->Clone();
		// 1. Start with an empty stack and an empty path
		std::stack<Node*> nodeStack;
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = graphCopy.GetActiveNodes();
		
		eulerianity = IsEulerian();
		auto pCurrentNode = [this, &Nodes, eulerianity]() -> Node*
		{
			switch (eulerianity)
			{
			case Eulerianity::eulerian:
				return Nodes[0];
			case Eulerianity::semiEulerian:
				{
					auto const OddPair = GetOddPair();
					return OddPair->first;
				}
			default:
				return nullptr;
			}
		}();
		if (pCurrentNode == nullptr) return {};
		
		while (true)
		{
			// https://web.archive.org/web/20240920214900/https://www.graph-magics.com/articles/euler.php
			
			auto const Neighbors = graphCopy.FindConnectionsWith(pCurrentNode->GetId());
			// 3. Repeat step 2 until the current vertex has no more neighbors and the stack is empty.
			if (Neighbors.empty() && nodeStack.empty())
			{
				break;
			}
			
			// 2. If current vertex has no neighbors - add it to circuit, remove the last vertex from the stack
			// and set it as the current one.
			if (Neighbors.empty())
			{
				Path.emplace_back(pCurrentNode);
				pCurrentNode = nodeStack.top();
				nodeStack.pop();
				continue;
			}
			
			// Otherwise (in case it has neighbors) - add the vertex to the stack,
			nodeStack.emplace(pCurrentNode);
			
			// take any of its neighbors, remove the edge between selected neighbor and that vertex,
			auto const *pConnection{Neighbors.front()};
			graphCopy.RemoveConnection(pConnection);
			
			// and set that neighbor as the current vertex.
			pCurrentNode = graphCopy.GetNode(pConnection->GetToId());
		}
		
		// "Also add the last currentnode to the path (after the while loop)" - Slides, 2026 colorized
		Path.emplace_back(pCurrentNode);
		
		// Keep in mind that we need the GraphNodes from the original graph! Not the copy (same Id)
		for (auto *&node : Path)
		{
			node = m_pGraph->GetNode(node->GetId());
		}

		std::reverse(Path.begin(), Path.end());
		return Path;
	}

	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::unordered_set<int>& visited, Node *pStartNode ) const
	{
		// TODO Mark the visited node
		visited.emplace(pStartNode->GetId());
		
		auto const Connections = m_pGraph->FindConnectionsFrom(pStartNode->GetId());
		for (auto Index = 0; Index < Connections.size(); ++Index)
		{
			auto const *Connection = Connections[Index];
			
			if (visited.contains(Connection->GetToId())) continue;
			VisitAllNodesDFS(Nodes, visited, m_pGraph->GetNode(Connection->GetToId()));
		}
		
		// TODO Ask the graph for the connections from that node
		// TODO recursively visit any valid connected nodes that were not visited before
		// TODO Tip: use an index-based for-loop to find the correct index
	}

	inline bool EulerianPath::GetIsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return false;

		// TODO choose a starting node
		
		auto const StartNode = Nodes[0];
		
		// TODO start a depth-first-search traversal from the node that has at least one connection
		std::unordered_set<int> visited{};
		VisitAllNodesDFS(Nodes, visited, StartNode);
		
		// TODO if a node was never visited, this graph is not connected
		return visited.size() == Nodes.size();
	}
}