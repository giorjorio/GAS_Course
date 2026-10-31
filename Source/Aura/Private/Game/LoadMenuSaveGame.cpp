// Giorjorio Copyright


#include "Game/LoadMenuSaveGame.h"

FSavedMap ULoadMenuSaveGame::GetSavedMapWithMapName(const FString& InMapName)
{
	for (const FSavedMap& Map : SavedMaps)
	{
		if (Map.MapAssetName == InMapName)
		{
			return Map;
		}
	}
	return FSavedMap();
}

bool ULoadMenuSaveGame::HasMap(const FString& InMapName)
{
	for (const FSavedMap& Map : SavedMaps)
	{
		if (Map.MapAssetName == InMapName)
		{
			return true;
		}
	}
	return false;
}

/*
bool ULoadMenuSaveGame::HasMap(const FString& InMapName) const
{
	return SavedMaps.ContainsByPredicate(
		[&InMapName](const FSavedMap& Map)
		{
			return Map.MapAssetName == InMapName;
		});
}

FSavedMap ULoadMenuSaveGame::GetSavedMapWithMapName(
	const FString& InMapName) const
{
	const FSavedMap* FoundMap = SavedMaps.FindByPredicate(
		[&InMapName](const FSavedMap& Map)
		{
			return Map.MapAssetName == InMapName;
		});

	return FoundMap ? *FoundMap : FSavedMap();
}
*/
