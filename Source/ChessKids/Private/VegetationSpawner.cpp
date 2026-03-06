// VegetationSpawner.cpp

#include "VegetationSpawner.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

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

	const FVector ActorOrigin = GetActorLocation();
	const FVector BoardCentre = FVector::ZeroVector; // chess board is at world origin

	for (const FVegetationEntry& Entry : SpawnEntries)
	{
		UStaticMesh* LoadedMesh = Entry.Mesh.LoadSynchronous();
		if (!LoadedMesh) continue;

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
			const float DistToBoard = FVector::Dist2D(SpawnLocation, BoardCentre);
			if (DistToBoard < ExclusionRadius)
				continue;

			// Random yaw
			if (bRandomYaw)
				SpawnRotation.Yaw = FMath::RandRange(0.f, 360.f);

			// Random scale
			const float Scale = FMath::RandRange(Entry.ScaleMin, Entry.ScaleMax);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation, Params);

			if (NewActor)
			{
				NewActor->GetStaticMeshComponent()->SetStaticMesh(LoadedMesh);
				NewActor->SetActorScale3D(FVector(Scale));
				NewActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
				SpawnedActors.Add(NewActor);
				Placed++;
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
