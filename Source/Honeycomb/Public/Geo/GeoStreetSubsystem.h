#pragma once

#include "CoreMinimal.h"
#include "Geo/GeoTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GeoStreetSubsystem.generated.h"

class AProceduralStreet;
class IHttpRequest;
class IHttpResponse;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreetBuilt, int32, BuildingCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreetFailed, const FString&, Reason);

/**
 * Builds a real-world street around the player at runtime.
 *
 * Flow: IP geolocation (ip-api.com) -> coordinates, then OpenStreetMap building
 * and road footprints (Overpass API), projected to local centimetres and handed
 * to AProceduralStreet. Everything is asynchronous over the engine HTTP module
 * and every entry point is Blueprint callable, so this can be driven from a
 * Blueprint graph as well as from C++.
 */
UCLASS()
class HONEYCOMB_API UGeoStreetSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Geolocates the user by their public IP, then builds the street there. */
    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Street")
    void BuildStreetFromUserLocation(float RadiusMeters = 250.0f);

    /** Builds a street at explicit coordinates, skipping IP geolocation. */
    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Street")
    void BuildStreetAt(double Latitude, double Longitude, float RadiusMeters = 250.0f);

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Street")
    bool IsBusy() const { return bBusy; }

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Street")
    FGeoLocation GetLastLocation() const { return LastLocation; }

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Street")
    AProceduralStreet* GetStreetActor() const { return StreetActor.Get(); }

    UPROPERTY(BlueprintAssignable, Category = "Honeycomb|Street")
    FOnStreetBuilt OnStreetBuilt;

    UPROPERTY(BlueprintAssignable, Category = "Honeycomb|Street")
    FOnStreetFailed OnStreetFailed;

private:
    void RequestIpLocation(uint32 RequestId);
    void HandleIpResponse(uint32 RequestId, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request, TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bConnectedOk);
    void RequestOverpass(uint32 RequestId);
    void HandleOverpassResponse(uint32 RequestId, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request, TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bConnectedOk);
    void RetryNextOverpassOrFail(uint32 RequestId, const FString& Reason);
    void SpawnStreet(const TArray<FGeoBuilding>& Buildings, const TArray<FGeoRoad>& Roads);
    void Fail(const FString& Reason);

    bool bBusy = false;

    /** Incremented every time a new build starts; stale async callbacks bail out on mismatch. */
    uint32 BuildRequestId = 0;

    FGeoLocation LastLocation;
    double OriginLatitude = 0.0;
    double OriginLongitude = 0.0;
    float SearchRadiusMeters = 250.0f;

    /** Attempt counter for the current Overpass request (transient server errors are retried). */
    int32 OverpassAttempt = 0;

    UPROPERTY(Transient)
    TObjectPtr<AProceduralStreet> StreetActor;

    TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> PendingRequest;
};
