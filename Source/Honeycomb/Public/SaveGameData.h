#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameData.generated.h"

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API USaveGameData: public USaveGame
{
    GENERATED_BODY()

    public:
        USaveGameData();

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        int32 ChakraLevel = 1;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        int32 Dimension = 3;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        int32 WorldIndex = 0;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        bool bIsDead = false;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        float TotalPlayTime = 0.0f;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        TArray<int32> UnlockedChakras;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        TArray<int32> CompletedWorlds;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        int32 CurrentKarma = 0;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        int32 ReincarnationCount = 0;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "SaveData")
        FDateTime LastSaveTime;

        void ResetToDefaults();
};
