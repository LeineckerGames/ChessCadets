// VegetationSpawner.cpp

#include "VegetationSpawner.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

AVegetationSpawner::AVegetationSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVegetationSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnMeshes();
}

void AVegetationSpawner::SpawnMeshes()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Clear any previously spawned meshes first
	ClearSpawnedMeshes();

	// Make sure there's a root to attach instanced-mesh components to.
	USceneComponent* Root = GetRootComponent();
	if (!Root)
	{
		Root = NewObject<USceneComponent>(this, TEXT("SpawnerRoot"));
		Root->RegisterComponent();
		SetRootComponent(Root);
	}

	const FVector ActorOrigin = GetActorLocation();
	const FVector BoardCentre = FVector::ZeroVector; // chess board is at world origin

	for (const FVegetationEntry& Entry : SpawnEntries)
	{
		UStaticMesh* LoadedMesh = Entry.Mesh.LoadSynchronous();
		if (!LoadedMesh) continue;

		// Static vegetation is batched into one HISM per entry (a single draw call
		// instead of up to 500 individual actors). An actor override (e.g. a moving
		// hover-car BP) still spawns real actors so they can keep moving.
		UHierarchicalInstancedStaticMeshComponent* HISM = nullptr;
		if (!SpawnActorClassOverride)
		{
			HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
			HISM->SetStaticMesh(LoadedMesh);
			// Must match the root's mobility — a Static child can't attach to a
			// Movable root (the "AttachTo ... Aborting" PIE errors).
			HISM->SetMobility(Root->Mobility);
			HISM->SetupAttachment(Root);
			HISM->RegisterComponent();
			SpawnedHISMs.Add(HISM);
		}

		int32 Placed = 0;
		int32 TotalAttempts = 0;
		const int32 MaxTotalAttempts = Entry.Count * MaxPlacementAttempts;

		while (Placed < Entry.Count && TotalAttempts < MaxTotalAttempts)
		{
			TotalAttempts++;

			FVector SpawnLocation;
			FRotator SpawnRotation;

			if (!TryGetGroundLocation(ActorOrigin, SpawnRadius, SpawnLocation, SpawnRotation))
				continue;

			// Skip if inside the chess board exclusion zone
			if (FVector::Dist2D(SpawnLocation, BoardCentre) < ExclusionRadius)
				continue;

			if (bRandomYaw)
				SpawnRotation.Yaw = FMath::RandRange(0.f, 360.f);

			const float Scale = FMath::RandRange(Entry.ScaleMin, Entry.ScaleMax);
			const FVector FinalLocation = SpawnLocation + FVector(0.f, 0.f, ZOffset);

			if (HISM)
			{
				// World-space instance — fast, batched, no per-mesh actor overhead.
				HISM->AddInstance(FTransform(SpawnRotation, FinalLocation, FVector(Scale)), /*bWorldSpace=*/true);
				Placed++;
			}
			else
			{
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AActor* NewActor = World->SpawnActor<AActor>(SpawnActorClassOverride.Get(), FinalLocation, SpawnRotation, Params);
				if (NewActor)
				{
					if (UStaticMeshComponent* SMC = NewActor->FindComponentByClass<UStaticMeshComponent>())
						SMC->SetStaticMesh(LoadedMesh);
					NewActor->SetActorScale3D(FVector(Scale));
					SpawnedActors.Add(NewActor);
					Placed++;
				}
			}
		}
	}
}

void AVegetationSpawner::ClearSpawnedMeshes()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
			Actor->Destroy();
	}
	SpawnedActors.Empty();

	for (UHierarchicalInstancedStaticMeshComponent* HISM : SpawnedHISMs)
	{
		if (IsValid(HISM))
		{
			HISM->ClearInstances();
			HISM->DestroyComponent();
		}
	}
	SpawnedHISMs.Empty();
}

bool AVegetationSpawner::TryGetGroundLocation(const FVector& Origin, float Radius, FVector& OutLocation, FRotator& OutRotation) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	// Pick a random point within the spawn radius
	const float Angle = FMath::RandRange(0.f, 360.f) * (PI / 180.f);
	const float Dist  = FMath::RandRange(0.f, Radius);
	const FVector RandomOffset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
	const FVector TraceStart = Origin + RandomOffset + FVector(0.f, 0.f, 5000.f);
	const FVector TraceEnd   = Origin + RandomOffset - FVector(0.f, 0.f, 5000.f);

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		OutLocation = Hit.ImpactPoint;

		if (bAlignToSurface)
			OutRotation = FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator();
		else
			OutRotation = FRotator::ZeroRotator;

		return true;
	}

	return false;
}
