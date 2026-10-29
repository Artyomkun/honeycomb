#include "DiseaseCatalog.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UDiseaseCatalog::UDiseaseCatalog()
{
    Diseases.Empty();
    DiseaseIndexById.Empty();
    bIsInitialized = false;

    UE_LOG(LogTemp, Verbose, TEXT("UDiseaseCatalog: Constructor initialized"));
}

void UDiseaseCatalog::Load()
{
    Entries.Empty();

    FString JsonPath = FPaths::ProjectContentDir() + TEXT("Data/UDiseaseCatalog.json");

    if (!FPaths::FileExists(JsonPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("UDiseaseCatalog: JSON file not found at %s"), *JsonPath);
        return;
    }

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *JsonPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("UDiseaseCatalog: Failed to load JSON file"));
        return;
    }

    SetJsonData(JsonString);
}

void UDiseaseCatalog::SetJsonData(const FString& JsonString)
{
    Entries.Empty();
    CachedJson = JsonString;

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UDiseaseCatalog: Failed to parse JSON"));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* DiseaseArray;
    if (!JsonObject->TryGetArrayField(TEXT("diseases"), DiseaseArray))
    {
        UE_LOG(LogTemp, Warning, TEXT("UDiseaseCatalog: No 'diseases' array found"));
        return;
    }

    for (const TSharedPtr<FJsonValue>& Value : *DiseaseArray)
    {
        TSharedPtr<FJsonObject> DiseaseObj = Value->AsObject();
        if (!DiseaseObj.IsValid()) continue;

        FDiseaseEntry Entry;
        Entry.Id = DiseaseObj->GetStringField(TEXT("id"));
        Entry.Name = DiseaseObj->GetStringField(TEXT("name"));
        Entry.Type = DiseaseObj->GetStringField(TEXT("type"));
        Entry.Description = DiseaseObj->GetStringField(TEXT("description"));

        const TArray<TSharedPtr<FJsonValue>>* SymptomsArray;
        if (DiseaseObj->TryGetArrayField(TEXT("symptoms"), SymptomsArray))
        {
            for (const TSharedPtr<FJsonValue>& SymptomValue : *SymptomsArray)
            {
                Entry.Symptoms.Add(SymptomValue->AsString());
            }
        }

        Entries.Add(Entry);
    }

    UE_LOG(LogTemp, Log, TEXT("UDiseaseCatalog: Loaded %d diseases"), Entries.Num());
}

FDiseaseEntry UDiseaseCatalog::FindById(const FString& Id) const
{
    for (const FDiseaseEntry& Entry : Entries)
    {
        if (Entry.Id == Id)
        {
            return Entry;
        }
    }
    return FDiseaseEntry();
}

TArray<FDiseaseEntry> UDiseaseCatalog::FindByType(const FString& Type) const
{
    TArray<FDiseaseEntry> Result;
    for (const FDiseaseEntry& Entry : Entries)
    {
        if (Entry.Type == Type)
        {
            Result.Add(Entry);
        }
    }
    return Result;
}
