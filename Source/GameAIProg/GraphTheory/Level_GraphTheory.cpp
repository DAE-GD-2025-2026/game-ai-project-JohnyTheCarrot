// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_GraphTheory.h"

#include "Algorithms/EulerianPath.h"
#include "Shared/GameAISpectator.h"

using namespace GameAI;

// Sets default values
ALevel_GraphTheory::ALevel_GraphTheory()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_GraphTheory::BeginPlay()
{
	Super::BeginPlay();
	Renderer = GraphRenderer{GetWorld()};
	
	// Add the graph editor to our player
	if (PlayerController = Cast<APlayerController>(GetWorld()->GetFirstLocalPlayerFromController()->PlayerController); 
		GraphEditorClass && PlayerController)
	{
		PlayerGraphEditor = NewObject<UGraphEditorComponent>(PlayerController->GetPawn(), GraphEditorClass);
		PlayerGraphEditor->RegisterComponent();
		PlayerGraphEditor->SetEditedGraph(&Graph);
		PlayerGraphEditor->SetNodeFactory(&NodeFactory);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to get PlayerController from LocalPlayer or GraphEditorClass is null"))
		return;
	}
	
	// Make the view orthogonal for less perspective issues
	if (AGameAISpectator* Player = Cast<AGameAISpectator>(PlayerController->GetPawnOrSpectator()); Player)
	{
		Player->SetCameraProjection(ECameraProjectionMode::Orthographic);
	}
	
	// TODO Make the graph and a couple connected nodes here...
	auto const Id1 = Graph.AddNode(std::make_unique<Node>(FVector2D{0, 0}));
	auto const Id2 = Graph.AddNode(std::make_unique<Node>(FVector2D{0, 200}));
	Graph.AddConnection(Id1, Id2);
	
	auto const Id3 = Graph.AddNode(std::make_unique<Node>(FVector2D{200, 0}));
	auto const Id4 = Graph.AddNode(std::make_unique<Node>(FVector2D{200, 200}));
	Graph.AddConnection(Id3, Id4);
	Graph.AddConnection(Id1, Id4);
	Graph.AddConnection(Id2, Id4);
	Graph.AddConnection(Id1, Id3);
	Graph.AddConnection(Id2, Id3);
	
	// Spawn the Agent
	Agent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
	FVector{0,0,90}, FRotator::ZeroRotator);
	Agent->SetSteeringBehavior(&PathFollow);
	std::vector<Node*> const Trail;
	UpdateAgentPath(Trail);
}

void ALevel_GraphTheory::BeginDestroy()
{
	Super::BeginDestroy();
}

void ALevel_GraphTheory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#pragma region UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
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
		ImGui::Spacing();

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::End();
	}
#pragma endregion UI
	
	Renderer.RenderGraph(Graph);
	
	if (PlayerGraphEditor->HasGraphUpdated())
	{
		std::vector<Node*> const Trail;
		UpdateAgentPath(Trail);
	}
	// TODO Check if the graph has updated
	// TODO if so, run the EulerianPath algorithm
	// TODO if a path is found, have the agent follow it
}

void ALevel_GraphTheory::UpdateAgentPath(std::vector<Node*> const& Trail)
{
	std::vector<FVector2D> path{};
	
	// TODO convert Node vector to positions vector
	EulerianPath EulerPath{&Graph};
	Eulerianity PathEulerianity{};
	auto const PathNodes{EulerPath.FindPath(PathEulerianity)};
	path.reserve(PathNodes.size());
	std::transform(PathNodes.begin(), PathNodes.end(), std::back_inserter(path), [](Node const *Node)
	{
		return Node->GetPosition();
	});

	PathFollow.SetPath(path);
	if (path.size() > 0)
	{
		Agent->SetPosition(path[0]);
	}
}




