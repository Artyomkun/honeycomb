#pragma once

#include "CoreMinimal.h"
#include "GeoTypes.generated.h"

/** Result of an IP geolocation lookup (ip-api.com). */
USTRUCT(BlueprintType)
struct HONEYCOMB_API FGeoLocation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    double Latitude = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    double Longitude = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    FString City;

    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    FString Region;

    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    FString Country;

    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    FString Ip;

    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    bool bValid = false;
};

/** A building footprint, already projected to street-local centimetres. */
USTRUCT(BlueprintType)
struct HONEYCOMB_API FGeoBuilding
{
    GENERATED_BODY()

    /** Closed outline in local cm. X = east, Y = north, first point not repeated. */
    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    TArray<FVector2D> Footprint;

    /** Wall height in cm. */
    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    float Height = 800.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    FString Kind;
};

/** A road centreline, already projected to street-local centimetres. */
USTRUCT(BlueprintType)
struct HONEYCOMB_API FGeoRoad
{
    GENERATED_BODY()

    /** Polyline in local cm. X = east, Y = north. */
    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    TArray<FVector2D> Centerline;

    /** Full ribbon width in cm. */
    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    float Width = 600.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Honeycomb|Geo")
    FString Kind;
};
