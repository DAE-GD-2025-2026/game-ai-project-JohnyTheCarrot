#pragma once
#include <vector>


class TriPolygon
{
public:
	
	struct Edge
	{
		std::array<int, 2> EdgeIndices;
		
		bool operator==(const Edge& Other) const;
		
		FVector GetP1(TriPolygon const& Poly) const { return Poly.Vertices[EdgeIndices[0]]; }
		FVector GetP2(TriPolygon const& Poly) const { return Poly.Vertices[EdgeIndices[1]]; }
		FVector GetMidPoint(TriPolygon const &Poly) const
		{
			auto const Edge1Point = GetP1(Poly);
			auto const Edge2Point = GetP2(Poly);
			return FMath::Lerp(Edge1Point, Edge2Point, 0.5f);
		}
	};
	
	struct Triangle
	{
		std::array<int, 3> VertexIndices;
		
		FVector GetVertex0(TriPolygon const& Poly) const { return Poly.Vertices[VertexIndices[0]]; }
		FVector GetVertex1(TriPolygon const& Poly) const { return Poly.Vertices[VertexIndices[1]]; }
		FVector GetVertex2(TriPolygon const& Poly) const { return Poly.Vertices[VertexIndices[2]]; }
		
		std::array<FVector, 3> GetVertices(TriPolygon const& Poly) const;
		std::vector<int> GetNeighbors(TriPolygon const& Poly) const;
		std::array<Edge, 3> GetEdges() const; // helper
		
		bool operator==(const Triangle& Other) const;
		bool Equals(const TArray<FVector>& VertexData, TriPolygon const & Poly) const;
		bool HasEdge(Edge const & Edge) const;
		
		void DebugDraw(const UWorld* World, TriPolygon const & Poly, FColor const & Color) const;
	};
	
	TriPolygon() = default;
	
	int AddTriangle(TArray<FVector> const & TriangleData);
	
	std::vector<FVector> const& GetVertices() const { return Vertices; }
	std::vector<Edge> const& GetEdges() const { return Edges; }
	std::vector<Triangle> const& GetTriangles() const { return Triangles; }
	Triangle const& GetTriangle(int TriIdx) const { return Triangles[TriIdx]; }
	
	void DrawDebug(UWorld const * World, FColor const & Color) const;
	
	// Queries
	std::vector<int> GetTriangleNeighbors(int InTriangleIndex) const;
	std::vector<int> GetTriangleNeighbors(Triangle const& InTriangle) const;
	
	std::optional<int> FindTriangleIndex(TArray<FVector> const& TriangleData) const;
	std::optional<int> FindVertexIndex(FVector const& Vertex) const;
	std::optional<int> FindEdgeIndex(Edge const& Edge) const;

	Triangle const* GetClosestTriangleToPosition(FVector2D const& DesiredPosition, FVector2D& OutPosition) const;
	Triangle const* GetTriangleAtPosition(FVector2D const& Position, bool OnLineAllowed) const;
	

private:
	int AddVertex(FVector const& Vertex);
	int AddEdge(Edge const & Edge);
	
	std::vector<FVector>  Vertices;
	std::vector<Edge> Edges;
	std::vector<Triangle> Triangles;
};

template<>
struct std::hash<TriPolygon::Edge>
{
	std::size_t operator()(const TriPolygon::Edge& Edge) const
	{
		std::size_t h1 = std::hash<int>{}(std::min(Edge.EdgeIndices[0], Edge.EdgeIndices[1]));
		std::size_t h2 = std::hash<int>{}(std::max(Edge.EdgeIndices[0], Edge.EdgeIndices[1]));
		h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
		return h1;
	}
};

template<>
struct std::hash<TriPolygon::Triangle>
{
	std::size_t operator()(const TriPolygon::Triangle& Tri) const
	{
		// Sort so {A,B,C} in any order hashes the same
		int indices[3] = { Tri.VertexIndices[0], Tri.VertexIndices[1], Tri.VertexIndices[2] };
		std::sort(std::begin(indices), std::end(indices));

		std::size_t seed = 0;
		for (int idx : indices)
		{
			seed ^= std::hash<int>{}(idx) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};