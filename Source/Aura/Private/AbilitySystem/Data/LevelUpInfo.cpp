// Giorjorio Copyright


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXp(int32 XP)
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		// LevelUpInformation[0] is meaningless, just a placeholder
		// LevelUpInformation[1] = Level 1 Information
		// LevelUpInformation[2] = Level 2 Information
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		if (XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			++Level;
		}
		else
		{
			bSearching = false;
		}
	}
	return Level;
	// can be done with for loop
}
