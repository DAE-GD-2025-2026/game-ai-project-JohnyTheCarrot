#pragma once

// Toggle this define to enable/disable spatial partitioning
// #define GAMEAI_USE_SPACE_PARTITIONING

#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "Movement/SteeringBehaviors/SteeringHelpers.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include <memory>
#include "imgui.h"
#include "INeighborAnalysis.h"
#include "Movement/SteeringBehaviors/Flocking/FlockingSteeringBehaviors.h"

class Flock final
{
public:
	Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize = 10, 
	float WorldSize = 100.f, 
	ASteeringAgent* const pAgentToEvade = nullptr, 
	bool bTrimWorld = false);

	~Flock();

	void Tick(float DeltaTime);
	void RenderDebug() const;
	void ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize);

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	//const TArray<ASteeringAgent*>& GetNeighbors() const { return pPartitionedSpace->GetNeighbors(); }
	//int GetNrOfNeighbors() const { return pPartitionedSpace->GetNrOfNeighbors(); }
#else // No space partitioning
	void RegisterNeighbors(ASteeringAgent* const Agent);
	int GetNrOfNeighbors() const { return NrOfNeighbors; }
#endif // USE_SPACE_PARTITIONING

	void UpdateNeighborList();

	void SetTarget_Seek(FSteeringParams const & Target);

private:
	// For debug rendering purposes
	UWorld* pWorld{nullptr};
	struct FlockBehavior final
	{
		std::unique_ptr<Cohesion> pCohesion;
		std::unique_ptr<Separation> pSeparation;
		std::unique_ptr<Alignment> pAlignment;
		std::unique_ptr<Wander> pWander{std::make_unique<Wander>()};
		std::unique_ptr<Seek> pSeek{std::make_unique<Seek>()};
		std::unique_ptr<BlendedSteering> pBlendedSteering{[this]
		{
			using Behavior = BlendedSteering::WeightedBehavior;
			return std::make_unique<BlendedSteering>(BlendedSteering{
				Behavior{pCohesion.get(), .9f},
				Behavior{pSeparation.get(), .8f},
				Behavior{pAlignment.get(), .7f},
				Behavior{pWander.get(), .5f},
				Behavior{pSeek.get(), .5f},
			});
		}()};
		std::unique_ptr<Evade> pEvade{std::make_unique<Evade>()};
		std::unique_ptr<PrioritySteering> pPrioritySteering{std::make_unique<PrioritySteering>(PrioritySteering{
			pEvade.get(),
			pBlendedSteering.get()
		})};
		
		explicit FlockBehavior(Flock const &Flock)
			: pCohesion{std::make_unique<Cohesion>(Flock)} 
			, pSeparation{std::make_unique<Separation>(Flock)}
			, pAlignment{std::make_unique<Alignment>(Flock)}
		{}
		
		[[nodiscard]]
		ISteeringBehavior *GetBehavior() const noexcept
		{
			return pPrioritySteering.get();
		}
	};
	
	int FlockSize{0};
	std::vector<ASteeringAgent*> Agents{};
	TUniquePtr<FlockBehavior> pBehaviors{MakeUnique<FlockBehavior>(*this)};

	std::vector<FFlockAgentNeighborInfo> Neighbors{};
	
	FNaiveNeighborAnalysis NaiveNeighborAnalysis{};
	FGridNeighborAnalysis GridNeighborAnalysis;
	
	int NeighborhoodAnalysisMethodIndex{1};
	
	[[nodiscard]]
	INeighborAnalysis *GetNeighborhoodAnalysisMethod();
	
	[[nodiscard]]
	INeighborAnalysis const *GetNeighborhoodAnalysisMethod() const;
	
	float NeighborhoodRadius{200.f};
	int NrOfNeighbors{0};

	ASteeringAgent* pAgentToEvade{nullptr};

	// UI and rendering
	bool DebugRenderSteering{false};
	bool DebugRenderNeighborhood{true};
	bool DebugRenderPartitions{true};

	void RenderNeighborhood();
};
