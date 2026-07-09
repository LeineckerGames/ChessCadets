#include "ChessKidsGameInstance.h"
#include "ChessKidsSaveGame.h"
#include "ChessMapRegistry.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/PackageName.h"

UChessKidsGameInstance::UChessKidsGameInstance()
{
	// Default the registry to the conventional asset path so OpenMap works without
	// a Blueprint subclass. Resolves silently once DA_MapRegistry is created.
	static ConstructorHelpers::FObjectFinder<UChessMapRegistry> RegistryFinder(
		TEXT("/Game/Data/DA_MapRegistry.DA_MapRegistry"));
	if (RegistryFinder.Succeeded())
	{
		MapRegistry = RegistryFinder.Object;
	}
}

static const TCHAR* ProgressSlot = TEXT("ChessKidsProgress");

void UChessKidsGameInstance::LoadProgress()
{
	if (UGameplayStatics::DoesSaveGameExist(ProgressSlot, 0))
		Progress = Cast<UChessKidsSaveGame>(UGameplayStatics::LoadGameFromSlot(ProgressSlot, 0));
	if (!Progress)
		Progress = Cast<UChessKidsSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UChessKidsSaveGame::StaticClass()));
}

void UChessKidsGameInstance::SaveProgress()
{
	if (Progress)
		UGameplayStatics::SaveGameToSlot(Progress, ProgressSlot, 0);
}

int32 UChessKidsGameInstance::StoryChapterForMap(const FString& MapName)
{
	if (MapName.Contains(TEXT("L_Pawn")))    return 1;
	if (MapName.Contains(TEXT("L_Knight")))  return 2;
	if (MapName.Contains(TEXT("L_Bishop")))  return 3;
	if (MapName.Contains(TEXT("L_Rook")))    return 4;
	if (MapName.Contains(TEXT("L_Royalty"))) return 5;
	return 0;
}

bool UChessKidsGameInstance::IsStoryChapterUnlocked(int32 Chapter) const
{
	if (bDevUnlockAll) return true;
	const int32 Unlocked = Progress ? Progress->UnlockedStoryChapters : 1;
	return Chapter >= 1 && Chapter <= Unlocked;
}

void UChessKidsGameInstance::NotifyStoryVictory(const FString& MapName)
{
	const int32 Chapter = StoryChapterForMap(MapName);
	if (Chapter <= 0 || !Progress) return;

	const int32 Next = FMath::Clamp(Chapter + 1, 1, 5);
	if (Next > Progress->UnlockedStoryChapters)
	{
		Progress->UnlockedStoryChapters = Next;
		SaveProgress();
	}
}

void UChessKidsGameInstance::MarkTutorialComplete(int32 LessonIndex)
{
	if (!Progress) return;
	if (!Progress->CompletedTutorials.Contains(LessonIndex))
	{
		Progress->CompletedTutorials.Add(LessonIndex);
		SaveProgress();
	}
}

bool UChessKidsGameInstance::IsTutorialComplete(int32 LessonIndex) const
{
	return Progress && Progress->CompletedTutorials.Contains(LessonIndex);
}

void UChessKidsGameInstance::Init()
{
	Super::Init();

	LoadProgress();

	if (UChessMapRegistry* Registry = LoadRegistry())
	{
		for (const FChessMapEntry& Entry : Registry->Maps)
		{
			if (Entry.bUnlockedByDefault && !Entry.MapId.IsNone())
			{
				UnlockedMaps.Add(Entry.MapId);
			}
		}
	}
}

UChessMapRegistry* UChessKidsGameInstance::LoadRegistry() const
{
	if (MapRegistry.IsNull())
	{
		return nullptr;
	}
	return MapRegistry.LoadSynchronous();
}

bool UChessKidsGameInstance::OpenMap(FName MapId)
{
	UChessMapRegistry* Registry = LoadRegistry();
	if (!Registry)
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenMap: no MapRegistry assigned on GameInstance."));
		return false;
	}

	FChessMapEntry Entry;
	if (!Registry->FindMap(MapId, Entry))
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenMap: unknown map id '%s'."), *MapId.ToString());
		return false;
	}

	if (!IsMapUnlocked(MapId))
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenMap: map '%s' is locked."), *MapId.ToString());
		return false;
	}

	OpenLevelAsset(Entry.Level);
	return true;
}

void UChessKidsGameInstance::OpenLevelAsset(const TSoftObjectPtr<UWorld>& Level)
{
	if (Level.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenLevelAsset: level reference is empty."));
		return;
	}

	const FString PackageName = FPackageName::ObjectPathToPackageName(Level.ToString());
	if (PackageName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenLevelAsset: could not resolve package name from '%s'."), *Level.ToString());
		return;
	}

	UGameplayStatics::OpenLevel(this, FName(*PackageName));
}

bool UChessKidsGameInstance::IsMapUnlocked(FName MapId) const
{
	return UnlockedMaps.Contains(MapId);
}

void UChessKidsGameInstance::UnlockMap(FName MapId)
{
	if (!MapId.IsNone())
	{
		UnlockedMaps.Add(MapId);
	}
}
