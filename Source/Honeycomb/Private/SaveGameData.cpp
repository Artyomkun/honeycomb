#include "SaveGameData.h"

USaveGameData::USaveGameData()
{
    ResetToDefaults();
}

void USaveGameData::ResetToDefaults()
{
    ChakraLevel = 1;
    Dimension = 3;
    WorldIndex = 0;
    bIsDead = false;
    TotalPlayTime = 0.0f;
    CurrentKarma = 0;
    ReincarnationCount = 0;
    UnlockedChakras.Empty();
    CompletedWorlds.Empty();
    LastSaveTime = FDateTime::Now();
}
