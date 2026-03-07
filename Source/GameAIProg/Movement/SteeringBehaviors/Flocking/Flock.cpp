#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, GridNeighborAnalysis{pWorld, WorldSize * 2.f, WorldSize * 2.f, 350.f}
	, pAgentToEvade{pAgentToEvade}
{
	Agents.resize(FlockSize);
	Neighbors.resize(FlockSize);
	
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	auto const SpawnSize = WorldSize - 100;
	float Increment = SpawnSize * 2.f / FMath::Sqrt(static_cast<float>(FlockSize));
	FVector SpawnPos{-SpawnSize, -SpawnSize, 300.f};
	for (auto *&Agent : Agents)
	{
		Agent = pWorld->SpawnActor<ASteeringAgent>(
			AgentClass, SpawnPos, FRotator::ZeroRotator, ActorSpawnParams
		);
		ensure(Agent != nullptr);
		
		Agent->IsDirected = true;
		Agent->PrimaryActorTick.bCanEverTick = false;
		
		SpawnPos.X += Increment;
		if (SpawnPos.X > SpawnSize)
		{
			SpawnPos.Y += Increment;
			SpawnPos.X = -SpawnSize;
		}
		
		Agent->SetDebugRenderingEnabled(false);
		
		Agent->SetSteeringBehavior(pBehaviors->GetBehavior());
	}
	Agents[0]->SetDebugRenderingEnabled(true);
	GetNeighborhoodAnalysisMethod()->Awaken(Agents);
}

Flock::~Flock()
{
	for (auto *const Agent : Agents)
	{
		Agent->Destroy();
	}
}

void Flock::Tick(float DeltaTime)
{
    // TODO: trim the agent to the world
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("Flock Tick");
	
	GetNeighborhoodAnalysisMethod()->Analyse(Neighbors, Agents, NeighborhoodRadius);
	
	pBehaviors->pEvade->SetTarget(pAgentToEvade);
	
	for (size_t index = 0; index < Agents.size(); ++index)
	{
		if (Neighbors[index].NumNeighbors > 0)
		{
			FTargetData Target;
			Target.Position = Neighbors[index].AveragePos / Neighbors[index].NumNeighbors;
			pBehaviors->pCohesion->SetTarget(Target);
		
			Target.LinearVelocity = Neighbors[index].CumulativeSeparationVec / Neighbors[index].SeparationWeightTotal;
			pBehaviors->pSeparation->SetTarget(Target);
		
			Target.LinearVelocity = Neighbors[index].AverageVelocity / Neighbors[index].NumNeighbors;
			pBehaviors->pAlignment->SetTarget(Target);
		}
		
		Agents[index]->PerformSteer(DeltaTime);
	}
}

void Flock::RenderDebug() const
{
	GetNeighborhoodAnalysisMethod()->DebugDraw();
	return;
	
	for (size_t index = 0; index < Agents.size(); ++index)
	{
		auto &Agent = *Agents[index];
		
		FColor const Color = Neighbors[index].NumNeighbors > 0 ? FColor::Green : FColor::Silver;
		
		Agent.DebugCircleFrom(NeighborhoodRadius, Color);
	}
	// GridPartitioning->DebugDraw();
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();
		
		std::string_view const ComboLabel{"Partitioning"};
		std::string_view const ComboOptions{"No partitioning\0Flat partitioning\0"};
		if (ImGui::Combo(ComboLabel.data(), &NeighborhoodAnalysisMethodIndex, ComboOptions.data()))
		{
			GetNeighborhoodAnalysisMethod()->Awaken(Agents);
		}

  // TODO: implement ImGUI checkboxes for debug rendering here

		auto constexpr SliderMax{2.f};
		ImGui::Text("Behavior Weights");
		{
			auto *Weight = pBehaviors->pBlendedSteering->GetWeight(pBehaviors->pCohesion.get());
			ImGui::SliderFloat("Cohesion", Weight, 0.f, SliderMax);
		}
		{
			auto *Weight = pBehaviors->pBlendedSteering->GetWeight(pBehaviors->pSeparation.get());
			ImGui::SliderFloat("Separation", Weight, 0.f, SliderMax);
		}
		{
			auto *Weight = pBehaviors->pBlendedSteering->GetWeight(pBehaviors->pAlignment.get());
			ImGui::SliderFloat("Alignment", Weight, 0.f, SliderMax);
		}
		{
			auto *Weight = pBehaviors->pBlendedSteering->GetWeight(pBehaviors->pWander.get());
			ImGui::SliderFloat("Wander", Weight, 0.f, SliderMax);
		}
		{
			auto *Weight = pBehaviors->pBlendedSteering->GetWeight(pBehaviors->pSeek.get());
			ImGui::SliderFloat("Seek", Weight, 0.f, SliderMax);
		}
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		//End
		ImGui::End();
	}
#pragma endregion
}

INeighborAnalysis* Flock::GetNeighborhoodAnalysisMethod() 
{
	switch (NeighborhoodAnalysisMethodIndex)
	{
	case 0:
		return &NaiveNeighborAnalysis;
	case 1:
		return &GridNeighborAnalysis;
	default:
		check(false);
		return nullptr;
	}
}

INeighborAnalysis const* Flock::GetNeighborhoodAnalysisMethod() const
{
	switch (NeighborhoodAnalysisMethodIndex)
	{
	case 0:
		return &NaiveNeighborAnalysis;
	case 1:
		return &GridNeighborAnalysis;
	default:
		check(false);
		return nullptr;
	}
}

void Flock::RenderNeighborhood()
{
 // TODO: Debugrender the neighbors for the first agent in the flock
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
 // TODO: Implement
}
#endif

void Flock::UpdateNeighborList()
{
	for (auto &Neighbor : Neighbors)
		Neighbor = {};
	
	for (auto AgentIt = Agents.cbegin(); AgentIt != Agents.cend(); ++AgentIt)
	{
		if (AgentIt + 1 == Agents.cend()) break;
		
		auto const AgentIndex = std::distance(Agents.cbegin(), AgentIt);
		
		for (auto NeighborAgentIt = AgentIt + 1; NeighborAgentIt != Agents.end(); ++NeighborAgentIt)
		{
			auto const NeighborIndex = std::distance(Agents.cbegin(), NeighborAgentIt);
			
			auto const Distance = (*AgentIt)->GetHorizontalDistanceTo(*NeighborAgentIt);
			if (Distance > NeighborhoodRadius) continue;
			
			Neighbors[AgentIndex].AveragePos += (*NeighborAgentIt)->GetPosition();
			Neighbors[AgentIndex].CumulativeSeparationVec += ((*AgentIt)->GetPosition() - (*NeighborAgentIt)->GetPosition()).GetSafeNormal() / Distance;
			Neighbors[AgentIndex].SeparationWeightTotal += 1.f / Distance;
			Neighbors[AgentIndex].AverageVelocity += (*NeighborAgentIt)->GetLinearVelocity();
			++Neighbors[AgentIndex].NumNeighbors;
			
			Neighbors[NeighborIndex].AveragePos += (*AgentIt)->GetPosition();
			Neighbors[NeighborIndex].CumulativeSeparationVec += ((*NeighborAgentIt)->GetPosition() - (*AgentIt)->GetPosition()).GetSafeNormal() / Distance;
			Neighbors[NeighborIndex].SeparationWeightTotal += 1.f / Distance;
			Neighbors[NeighborIndex].AverageVelocity += (*AgentIt)->GetLinearVelocity();
			++Neighbors[NeighborIndex].NumNeighbors;
		}
	}
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	pBehaviors->pSeek->SetTarget(Target);
}
