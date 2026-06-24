#include "ChessMapRegistry.h"

bool UChessMapRegistry::FindMap(FName MapId, FChessMapEntry& OutEntry) const
{
	for (const FChessMapEntry& Entry : Maps)
	{
		if (Entry.MapId == MapId)
		{
			OutEntry = Entry;
			return true;
		}
	}
	return false;
}
