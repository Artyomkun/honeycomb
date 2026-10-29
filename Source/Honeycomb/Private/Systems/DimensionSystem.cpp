#include "DimensionSystem.h"
#include "GameManager.h"
#include "UI/UIDimensionDisplay.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/WorldSettings.h"

UDimensionSystem::UDimensionSystem() {}

void UDimensionSystem::InitializeDimensions()
{
    Dimensions.Empty();

    FDimensionData Dim1;
    Dim1.Level = 1;
    Dim1.Name = TEXT("1D - Физический Мир");
    Dim1.Description = TEXT("Базовый физический мир, где существуют только материальные объекты");
    Dim1.DimensionColor = FLinearColor::Gray;
    Dim1.RealityShift = 0.0f;
    Dim1.Features = { TEXT("Только материя"), TEXT("Базовые физические законы"), TEXT("Ограниченное восприятие") };
    Dim1.bIsUnlocked = true;
    Dim1.RequiredChakraLevel = 1;
    Dimensions.Add(Dim1);

    FDimensionData Dim2;
    Dim2.Level = 2;
    Dim2.Name = TEXT("2D - Плоский Мир");
    Dim2.Description = TEXT("Двумерный мир с ограниченным движением и восприятием");
    Dim2.DimensionColor = FLinearColor::Blue;
    Dim2.RealityShift = 0.1f;
    Dim2.Features = { TEXT("Двумерное движение"), TEXT("Плоские объекты"), TEXT("Ограниченная глубина") };
    Dim2.bIsUnlocked = false;
    Dim2.RequiredChakraLevel = 2;
    Dimensions.Add(Dim2);

    FDimensionData Dim3;
    Dim3.Level = 3;
    Dim3.Name = TEXT("3D - Наш Мир");
    Dim3.Description = TEXT("Трехмерный мир, который мы знаем");
    Dim3.DimensionColor = FLinearColor::White;
    Dim3.RealityShift = 0.2f;
    Dim3.Features = { TEXT("Полное 3D восприятие"), TEXT("Время как четвертое измерение"), TEXT("Физические законы") };
    Dim3.bIsUnlocked = true;
    Dim3.RequiredChakraLevel = 3;
    Dimensions.Add(Dim3);

    FDimensionData Dim4;
    Dim4.Level = 4;
    Dim4.Name = TEXT("4D - Временное Измерение");
    Dim4.Description = TEXT("Мир, где время становится управляемым измерением");
    Dim4.DimensionColor = FLinearColor(0.0f, 1.0f, 1.0f);
    Dim4.RealityShift = 0.5f;
    Dim4.Features = { TEXT("Управление временем"), TEXT("Видение прошлого и будущего"), TEXT("Временные петли") };
    Dim4.bIsUnlocked = false;
    Dim4.RequiredChakraLevel = 7;
    Dimensions.Add(Dim4);

    FDimensionData Dim5;
    Dim5.Level = 5;
    Dim5.Name = TEXT("5D - Духовное Измерение");
    Dim5.Description = TEXT("Высшее измерение, где душа и сознание преобладают над материей");
    Dim5.DimensionColor = FLinearColor(1.0f, 0.0f, 1.0f);
    Dim5.RealityShift = 1.0f;
    Dim5.Features = { TEXT("Создание реальности"), TEXT("Межпространственные путешествия"), TEXT("Единство с космосом") };
    Dim5.bIsUnlocked = false;
    Dim5.RequiredChakraLevel = 10;
    Dimensions.Add(Dim5);

    CurrentDimensionLevel = 3;
    UpdateUI(Dim3);
}

void UDimensionSystem::OnDimensionChange(int32 NewDimensionLevel)
{
    FDimensionData Dimension = GetDimensionData(NewDimensionLevel);
    if (Dimension.Level == 0 || !Dimension.bIsUnlocked) return;

    CurrentDimensionLevel = NewDimensionLevel;
    UpdateUI(Dimension);
    ApplyDimensionEffects(Dimension);
}

bool UDimensionSystem::IsDimensionUnlocked(int32 Level) const
{
    FDimensionData Dim = GetDimensionData(Level);
    return Dim.Level != 0 && Dim.bIsUnlocked;
}

FDimensionData UDimensionSystem::GetDimensionData(int32 Level) const
{
    for (const FDimensionData& Dim : Dimensions)
    {
        if (Dim.Level == Level) return Dim;
    }
    return FDimensionData();
}

void UDimensionSystem::ModifyRealityStability(float Amount)
{
    RealityStability = FMath::Clamp(RealityStability + Amount, 0.0f, 100.0f);
}

void UDimensionSystem::TriggerRealityShift()
{
    DimensionalShift += 0.1f;
    ModifyRealityStability(-5.0f);
}

void UDimensionSystem::StabilizeReality()
{
    DimensionalShift = FMath::Max(0.0f, DimensionalShift - 0.05f);
    ModifyRealityStability(2.0f);
}

void UDimensionSystem::ActivateShift(int32 NewDimensionLevel)
{
    if (!IsDimensionUnlocked(NewDimensionLevel))
    {
        UE_LOG(LogTemp, Warning, TEXT("UDimensionSystem: Dimension %d is not unlocked"), NewDimensionLevel);
        return;
    }

    OnDimensionChange(NewDimensionLevel);
    UE_LOG(LogTemp, Log, TEXT("UDimensionSystem: Dimension shift activated to level %d"), NewDimensionLevel);
}

void UDimensionSystem::ApplyDimensionEffects(const FDimensionData& Dimension)
{
    DimensionalShift = Dimension.RealityShift;
    ApplyPhysicsChanges(Dimension);
}

void UDimensionSystem::ApplyPhysicsChanges(const FDimensionData& Dimension)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("UDimensionSystem: No world context"));
        return;
    }

    if (AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings())
    {
        WorldSettings->GlobalGravityZ = Dimension.GravityScale * -980.0f;
        UE_LOG(LogTemp, Log, TEXT("UDimensionSystem: Gravity set to %f"), WorldSettings->GlobalGravityZ);
    }
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), Dimension.TimeDilation);
    UE_LOG(LogTemp, Log, TEXT("UDimensionSystem: Time dilation set to %f"), Dimension.TimeDilation);

    ApplyPhysicsMaterials(Dimension.PhysicsMaterial);

    OnPhysicsChanged.Broadcast(Dimension.Index);
}

void UDimensionSystem::ApplyPhysicsMaterials(UPhysicalMaterial* Material)
{
    if (!GetWorld())
    {
        return;
    }

    if (Material == nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("UDimensionSystem: No physics material to apply"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UDimensionSystem: Physics material applied: %s"), *GetNameSafe(Material));
}

void UDimensionSystem::UpdateUI(const FDimensionData& Dimension)
{
    if (!DimensionUI)
    {
        UE_LOG(LogTemp, Warning, TEXT("UDimensionSystem: DimensionUI not found"));
        return;
    }

    DimensionUI->SetDimensionName(Dimension.Name);
    DimensionUI->SetDimensionColor(Dimension.WorldColor);
    DimensionUI->SetDimensionIndex(Dimension.Index);

    if (Dimension.Index != CurrentDimensionIndex)
    {
        DimensionUI->PlayTransitionAnimation(Dimension.Index);
    }

    CurrentDimensionIndex = Dimension.Index;

    UE_LOG(LogTemp, Log, TEXT("UDimensionSystem: UI updated for dimension %d"), Dimension.Index);
}
