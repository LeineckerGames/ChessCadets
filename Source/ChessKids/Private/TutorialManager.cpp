#include "TutorialManager.h"
#include "TutorialBoard.h"
#include "ChessPiece.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

ATutorialManager::ATutorialManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

int32 ATutorialManager::PackPos(int32 File, int32 Rank) const
{
	return File * 100 + Rank;
}

void ATutorialManager::BeginPlay()
{
	Super::BeginPlay();

	// Auto-find the board if not set
	if (!Board)
	{
		for (TActorIterator<ATutorialBoard> It(GetWorld()); It; ++It)
		{
			Board = *It;
			break;
		}
	}

	if (Board)
	{
		SetupPhase(ETutorialPhase::Phase1);
	}
}

void ATutorialManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATutorialManager::ShowGuideMessage(const FString& Message)
{
	OnGuideMessage.Broadcast(Message);   // rendered by UTutorialHudWidget

	if (false)   // debug fallback retired — the guide panel owns messaging now
	{
		GEngine->AddOnScreenDebugMessage(/*Key=*/1, /*TimeToDisplay=*/5.f,
			FColor::Cyan, Message, /*bNewerOnTop=*/true, FVector2D(2.f, 2.f));
	}
}

void ATutorialManager::SpawnPlayerPiece(int32 File, int32 Rank)
{
	if (!Board) return;

	if (PlayerPiece)
	{
		PlayerPiece->Destroy();
		PlayerPiece = nullptr;
	}

	FVector Location = Board->GridToWorldLocation(File, Rank, 5.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FPieceMeshConfig* Config = PieceMeshes.Find(TaughtPiece);
	if (Config && Config->PieceClass)
	{
		PlayerPiece = GetWorld()->SpawnActor<AChessPiece>(Config->PieceClass, Location, FRotator::ZeroRotator, Params);
	}
	else
	{
		PlayerPiece = GetWorld()->SpawnActor<AChessPiece>(AChessPiece::StaticClass(), Location, FRotator::ZeroRotator, Params);
	}

	if (PlayerPiece)
	{
		PlayerPiece->Init(TaughtPiece, EChessColor::White, File, Rank, Config);
		PlayerFile = File;
		PlayerRank = Rank;
		bPlayerHasMoved = false;
	}
}

void ATutorialManager::MovePlayerTo(int32 File, int32 Rank)
{
	if (!Board || !PlayerPiece) return;

	Board->GlideActorToSquare(PlayerPiece, File, Rank, 5.f);
	PlayerFile = File;
	PlayerRank = Rank;
	PlayerPiece->BoardFile = File;
	PlayerPiece->BoardRank = Rank;
	bPlayerHasMoved = true;
	MoveCount++;
}

void ATutorialManager::SpawnTargetAt(int32 File, int32 Rank)
{
	if (!Board) return;

	const int32 Key = PackPos(File, Rank);
	if (TargetActors.Contains(Key)) return;

	FVector Location = Board->GridToWorldLocation(File, Rank, 5.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn a pawn as a visual target marker — can be replaced with a proper collectible mesh later
	AChessPiece* Target = GetWorld()->SpawnActor<AChessPiece>(AChessPiece::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Target)
	{
		Target->Init(EChessPieceType::Pawn, EChessColor::Black, File, Rank, nullptr);
		Target->SetActorScale3D(FVector(0.6f));
		TargetActors.Add(Key, Target);
		TargetPositions.Add(FIntPoint(File, Rank));
		TargetsTotal++;
	}
}

void ATutorialManager::RemoveTargetAt(int32 File, int32 Rank)
{
	const int32 Key = PackPos(File, Rank);
	if (AChessPiece** Found = TargetActors.Find(Key))
	{
		if (IsValid(*Found)) (*Found)->Destroy();
		TargetActors.Remove(Key);
	}
	TargetPositions.Remove(FIntPoint(File, Rank));
}

void ATutorialManager::SpawnEnemyAt(int32 File, int32 Rank)
{
	if (!Board) return;

	const int32 Key = PackPos(File, Rank);
	if (EnemyActors.Contains(Key)) return;

	FVector Location = Board->GridToWorldLocation(File, Rank, 5.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AChessPiece* Enemy = GetWorld()->SpawnActor<AChessPiece>(AChessPiece::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Enemy)
	{
		Enemy->Init(EChessPieceType::Pawn, EChessColor::Black, File, Rank, nullptr);
		EnemyActors.Add(Key, Enemy);
		EnemyPositions.Add(FIntPoint(File, Rank));
	}
}

void ATutorialManager::RemoveEnemyAt(int32 File, int32 Rank)
{
	const int32 Key = PackPos(File, Rank);
	if (AChessPiece** Found = EnemyActors.Find(Key))
	{
		if (IsValid(*Found)) (*Found)->Destroy();
		EnemyActors.Remove(Key);
	}
	EnemyPositions.Remove(FIntPoint(File, Rank));
}

void ATutorialManager::SpawnWallAt(int32 File, int32 Rank)
{
	if (!Board) return;

	const int32 Key = PackPos(File, Rank);
	if (WallActors.Contains(Key)) return;

	FVector Location = Board->GridToWorldLocation(File, Rank, 0.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* Wall = GetWorld()->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Wall)
	{
		Wall->SetMobility(EComponentMobility::Movable);
		if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
			Wall->GetStaticMeshComponent()->SetStaticMesh(Cube);
		const float S = Board->SquareSize / 100.f;
		Wall->SetActorScale3D(FVector(S * 0.9f, S * 0.9f, S * 0.5f));
		Wall->SetActorLocation(Location + FVector(0.f, 0.f, Board->SquareSize * 0.25f));
		WallActors.Add(Key, Wall);
	}
}

bool ATutorialManager::IsSquareBlocked(int32 File, int32 Rank) const
{
	const int32 Key = PackPos(File, Rank);
	return EnemyActors.Contains(Key) || WallActors.Contains(Key);
}

TArray<FIntPoint> ATutorialManager::GetLegalMoves() const
{
	TArray<FIntPoint> Moves;
	if (!Board) return Moves;

	for (int32 F = 0; F < Board->NumFiles; ++F)
	{
		for (int32 R = 0; R < Board->NumRanks; ++R)
		{
			if (F == PlayerFile && R == PlayerRank) continue;
			if (IsMoveAllowed(PlayerFile, PlayerRank, F, R))
				Moves.Add(FIntPoint(F, R));
		}
	}
	return Moves;
}

bool ATutorialManager::IsMoveAllowed(int32 FromFile, int32 FromRank, int32 ToFile, int32 ToRank) const
{
	if (!Board || !Board->IsValidSquare(ToFile, ToRank)) return false;

	// Walls can never be landed on
	if (WallActors.Contains(PackPos(ToFile, ToRank))) return false;

	const int32 FileDiff = FMath::Abs(ToFile - FromFile);
	const int32 RankDiff = ToRank - FromRank; // positive = forward (white moves up = rank increases)

	// Check if an enemy is on the target square
	const int32 TargetKey = PackPos(ToFile, ToRank);
	const bool bEnemyOnTarget = EnemyActors.Contains(TargetKey);

	// Check if a friendly piece or enemy blocks the path (for straight moves)
	const int32 BlockKey = PackPos(ToFile, ToRank);

	switch (TaughtPiece)
	{
	case EChessPieceType::Pawn:
	{
		// Forward one square (no capture)
		if (FileDiff == 0 && RankDiff == 1 && !bEnemyOnTarget)
			return true;
		// Forward two squares on first move (no capture, no blocking piece in between)
		if (FileDiff == 0 && RankDiff == 2 && !bPlayerHasMoved && !bEnemyOnTarget)
		{
			if (!IsSquareBlocked(FromFile, FromRank + 1)) return true;
		}
		// Diagonal capture
		if (FileDiff == 1 && RankDiff == 1 && bEnemyOnTarget)
			return true;
		return false;
	}
	case EChessPieceType::Rook:
	{
		if (FileDiff > 0 && RankDiff != 0) return false; // must be straight line
		if (FileDiff == 0 && RankDiff == 0) return false;
		// Check path is clear
		const int32 FileStep = (ToFile > FromFile) ? 1 : (ToFile < FromFile) ? -1 : 0;
		const int32 RankStep = (ToRank > FromRank) ? 1 : (ToRank < FromRank) ? -1 : 0;
		int32 CF = FromFile + FileStep, CR = FromRank + RankStep;
		while (CF != ToFile || CR != ToRank)
		{
			if (IsSquareBlocked(CF, CR)) return false;
			CF += FileStep; CR += RankStep;
		}
		return true;
	}
	case EChessPieceType::Bishop:
	{
		if (FileDiff != FMath::Abs(RankDiff)) return false; // must be diagonal
		if (FileDiff == 0) return false;
		const int32 FileStep = (ToFile > FromFile) ? 1 : -1;
		const int32 RankStep = (ToRank > FromRank) ? 1 : -1;
		int32 CF = FromFile + FileStep, CR = FromRank + RankStep;
		while (CF != ToFile || CR != ToRank)
		{
			if (IsSquareBlocked(CF, CR)) return false;
			CF += FileStep; CR += RankStep;
		}
		return true;
	}
	case EChessPieceType::Knight:
	{
		return (FileDiff == 2 && FMath::Abs(RankDiff) == 1) ||
		       (FileDiff == 1 && FMath::Abs(RankDiff) == 2);
	}
	case EChessPieceType::Queen:
	{
		// Rook-like or bishop-like
		const bool bStraight = (FileDiff == 0 || RankDiff == 0);
		const bool bDiagonal = (FileDiff == FMath::Abs(RankDiff));
		if (!bStraight && !bDiagonal) return false;
		if (FileDiff == 0 && RankDiff == 0) return false;
		const int32 FileStep = (ToFile > FromFile) ? 1 : (ToFile < FromFile) ? -1 : 0;
		const int32 RankStep = (ToRank > FromRank) ? 1 : (ToRank < FromRank) ? -1 : 0;
		int32 CF = FromFile + FileStep, CR = FromRank + RankStep;
		while (CF != ToFile || CR != ToRank)
		{
			if (IsSquareBlocked(CF, CR)) return false;
			CF += FileStep; CR += RankStep;
		}
		return true;
	}
	case EChessPieceType::King:
	{
		return FileDiff <= 1 && FMath::Abs(RankDiff) <= 1 && (FileDiff + FMath::Abs(RankDiff) > 0);
	}
	}
	return false;
}

void ATutorialManager::UpdateHighlights()
{
	if (!Board) return;
	Board->ClearHighlights();
	Board->SelectSquare(PlayerFile, PlayerRank);

	TArray<FIntPoint> Moves = GetLegalMoves();
	Board->ShowLegalMoves(Moves);
}

void ATutorialManager::OnSquareClicked(int32 File, int32 Rank)
{
	if (CurrentPhase == ETutorialPhase::Complete) return;
	if (bPhaseTransitionPending) return;
	if (!Board) return;

	if (!IsMoveAllowed(PlayerFile, PlayerRank, File, Rank))
	{
		ShowGuideMessage(TEXT("That square isn't reachable! Look at the glowing squares."));
		return;
	}

	// Check if we captured an enemy
	const int32 EnemyKey = PackPos(File, Rank);
	if (EnemyActors.Contains(EnemyKey))
	{
		RemoveEnemyAt(File, Rank);
	}

	MovePlayerTo(File, Rank);

	// Check if we collected a target
	const int32 TargetKey = PackPos(File, Rank);
	if (TargetActors.Contains(TargetKey))
	{
		RemoveTargetAt(File, Rank);
		TargetsCollected++;
		OnTargetCollected(File, Rank);
	}

	AfterPlayerMove(File, Rank);

	// If the move ended the level (e.g. boss win), clear the board instead of re-highlighting
	if (CurrentPhase == ETutorialPhase::Complete)
	{
		Board->ClearHighlights();
		return;
	}

	UpdateHighlights();

	// Check if phase is complete (all targets collected AND enemies cleared)
	if (TargetPositions.Num() == 0 && EnemyPositions.Num() == 0)
	{
		OnPhaseComplete();
		return;
	}

	// Soft-lock detection: if no legal moves remain, reset phase with hint
	if (GetLegalMoves().Num() == 0)
	{
		ShowGuideMessage(TEXT("Oops, no moves left! Let's try again. Remember: capture diagonally!"));
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ATutorialManager::ResetPhase, 2.0f, false);
	}
}

void ATutorialManager::AdvancePhase()
{
	switch (CurrentPhase)
	{
	case ETutorialPhase::Phase1: CurrentPhase = ETutorialPhase::Phase2; break;
	case ETutorialPhase::Phase2: CurrentPhase = ETutorialPhase::Phase3; break;
	case ETutorialPhase::Phase3: CurrentPhase = ETutorialPhase::Boss; break;
	case ETutorialPhase::Boss:   CurrentPhase = ETutorialPhase::Complete; break;
	default: return;
	}

	OnPhaseChanged.Broadcast(CurrentPhase);
	SetupPhase(CurrentPhase);
}

void ATutorialManager::ResetPhase()
{
	OnTryAgain.Broadcast();
	SetupPhase(CurrentPhase);
}

void ATutorialManager::SetupPhase(ETutorialPhase Phase)
{
	// Clear existing state
	for (auto& Pair : TargetActors)
		if (IsValid(Pair.Value)) Pair.Value->Destroy();
	TargetActors.Empty();
	TargetPositions.Empty();

	for (auto& Pair : EnemyActors)
		if (IsValid(Pair.Value)) Pair.Value->Destroy();
	EnemyActors.Empty();
	EnemyPositions.Empty();

	for (auto& Pair : WallActors)
		if (IsValid(Pair.Value)) Pair.Value->Destroy();
	WallActors.Empty();

	TargetsCollected = 0;
	TargetsTotal = 0;
	MoveCount = 0;
	bPlayerHasMoved = false;
	bPhaseTransitionPending = false;
}

void ATutorialManager::AfterPlayerMove(int32 File, int32 Rank)
{
	// Base does nothing — subclasses override for AI turns, etc.
}

void ATutorialManager::OnTargetCollected(int32 File, int32 Rank)
{
	// Base implementation — subclasses can override for custom feedback
}

void ATutorialManager::OnPhaseComplete()
{
	if (CurrentPhase == ETutorialPhase::Boss)
	{
		CurrentPhase = ETutorialPhase::Complete;
		OnLevelComplete.Broadcast();
		ShowGuideMessage(TEXT("Amazing! You've mastered this piece!"));

		// Chain to the next lesson after the celebration has a moment on screen.
		if (NextLevelName != NAME_None)
		{
			TWeakObjectPtr<ATutorialManager> WeakThis(this);
			FTimerHandle TravelHandle;
			GetWorldTimerManager().SetTimer(TravelHandle, [WeakThis]()
			{
				if (ATutorialManager* Self = WeakThis.Get())
					UGameplayStatics::OpenLevel(Self, Self->NextLevelName);
			}, 3.5f, false);
		}
	}
	else
	{
		ShowGuideMessage(TEXT("Great job! Ready for the next challenge?"));
		// Auto-advance after a short delay — or let UI trigger AdvancePhase()
		bPhaseTransitionPending = true;
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ATutorialManager::AdvancePhase, 2.0f, false);
	}
}
