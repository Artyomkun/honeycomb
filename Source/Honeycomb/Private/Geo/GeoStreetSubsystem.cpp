#include "Geo/GeoStreetSubsystem.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Geo/ProceduralStreet.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace GeoStreetPrivate
{
    constexpr double MetersPerDegreeLatitude = 111320.0;

    /** Local centimetres for a lat/lon relative to the street origin. X = east, Y = north. */
    FVector2D ProjectToLocalCm(double Lat, double Lon, double OriginLat, double OriginLon)
    {
        const double MetersPerDegreeLongitude =
            MetersPerDegreeLatitude * FMath::Cos(FMath::DegreesToRadians(OriginLat));
        const double EastMeters = (Lon - OriginLon) * MetersPerDegreeLongitude;
        const double NorthMeters = (Lat - OriginLat) * MetersPerDegreeLatitude;
        return FVector2D(EastMeters * 100.0, NorthMeters * 100.0);
    }

    float RoadWidthCm(const FString& Highway)
    {
        if (Highway.Contains(TEXT("motorway"))) return 1400.0f;
        if (Highway.Contains(TEXT("trunk"))) return 1200.0f;
        if (Highway.Contains(TEXT("primary"))) return 1000.0f;
        if (Highway.Contains(TEXT("secondary"))) return 800.0f;
        if (Highway.Contains(TEXT("tertiary"))) return 700.0f;
        if (Highway.Contains(TEXT("residential"))) return 600.0f;
        if (Highway.Contains(TEXT("living_street"))) return 500.0f;
        if (Highway.Contains(TEXT("service"))) return 400.0f;
        if (Highway.Contains(TEXT("pedestrian"))) return 500.0f;
        if (Highway.Contains(TEXT("footway"))) return 250.0f;
        if (Highway.Contains(TEXT("path"))) return 200.0f;
        if (Highway.Contains(TEXT("cycleway"))) return 250.0f;
        return 500.0f;
    }

    float ParseLeadingFloat(const FString& In, float Fallback)
    {
        FString Cleaned;
        Cleaned.Reserve(In.Len());
        for (const TCHAR Ch : In)
        {
            if (FChar::IsDigit(Ch) || Ch == TEXT('.') || Ch == TEXT(',') || Ch == TEXT('-'))
            {
                Cleaned.AppendChar(Ch == TEXT(',') ? TEXT('.') : Ch);
            }
            else if (Cleaned.Len() > 0)
            {
                break;
            }
        }
        return Cleaned.IsEmpty() ? Fallback : FCString::Atof(*Cleaned);
    }
}

void UGeoStreetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[Honeycomb] GeoStreetSubsystem ready"));
}

void UGeoStreetSubsystem::BuildStreetFromUserLocation(float RadiusMeters)
{
    const uint32 RequestId = ++BuildRequestId;
    bBusy = true;
    SearchRadiusMeters = FMath::Clamp(RadiusMeters, 50.0f, 1000.0f);
    OverpassAttempt = 0;
    RequestIpLocation(RequestId);
}

void UGeoStreetSubsystem::BuildStreetAt(double Latitude, double Longitude, float RadiusMeters)
{
    const uint32 RequestId = ++BuildRequestId;
    bBusy = true;
    SearchRadiusMeters = FMath::Clamp(RadiusMeters, 50.0f, 1000.0f);
    OverpassAttempt = 0;

    OriginLatitude = Latitude;
    OriginLongitude = Longitude;
    LastLocation = FGeoLocation();
    LastLocation.Latitude = Latitude;
    LastLocation.Longitude = Longitude;
    LastLocation.bValid = true;

    RequestOverpass(RequestId);
}

void UGeoStreetSubsystem::RequestIpLocation(uint32 RequestId)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("http://ip-api.com/json/"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("Honeycomb/1.0 (Unreal)"));
    Request->SetTimeout(30.0f);
    const uint32 CapturedRequestId = RequestId;
    Request->OnProcessRequestComplete().BindWeakLambda(
        this, [this, CapturedRequestId](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk)
        {
            HandleIpResponse(CapturedRequestId, MoveTemp(Req), MoveTemp(Resp), bOk);
        });

    PendingRequest = Request;
    UE_LOG(LogTemp, Log, TEXT("[Honeycomb] Geolocating user by IP..."));
    Request->ProcessRequest();
}

void UGeoStreetSubsystem::HandleIpResponse(
    uint32 RequestId,
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
    TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
    bool bConnectedOk)
{
    if (RequestId != BuildRequestId)
    {
        return;
    }

    if (!bConnectedOk || !Response.IsValid() || Response->GetResponseCode() != 200)
    {
        Fail(TEXT("IP geolocation request failed"));
        return;
    }

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader =
        TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        Fail(TEXT("IP geolocation response was not valid JSON"));
        return;
    }

    FString Status;
    if (Root->TryGetStringField(TEXT("status"), Status) && Status != TEXT("success"))
    {
        Fail(FString::Printf(TEXT("IP geolocation rejected: %s"), *Status));
        return;
    }

    double Lat = 0.0;
    double Lon = 0.0;
    if (!Root->TryGetNumberField(TEXT("lat"), Lat) || !Root->TryGetNumberField(TEXT("lon"), Lon))
    {
        Fail(TEXT("IP geolocation response had no coordinates"));
        return;
    }

    OriginLatitude = Lat;
    OriginLongitude = Lon;

    LastLocation = FGeoLocation();
    LastLocation.Latitude = Lat;
    LastLocation.Longitude = Lon;
    LastLocation.bValid = true;
    Root->TryGetStringField(TEXT("city"), LastLocation.City);
    Root->TryGetStringField(TEXT("regionName"), LastLocation.Region);
    Root->TryGetStringField(TEXT("country"), LastLocation.Country);
    Root->TryGetStringField(TEXT("query"), LastLocation.Ip);

    UE_LOG(LogTemp, Log, TEXT("[Honeycomb] IP location: %s, %s (%.5f, %.5f) ip=%s"),
        *LastLocation.City, *LastLocation.Country, Lat, Lon, *LastLocation.Ip);

    RequestOverpass(RequestId);
}

void UGeoStreetSubsystem::RequestOverpass(uint32 RequestId)
{
    const FString Query = FString::Printf(
        TEXT("[out:json][timeout:60];(")
        TEXT("way(around:%d,%.6f,%.6f)[building];")
        TEXT("way(around:%d,%.6f,%.6f)[highway]")
        TEXT("[highway!~\"^(footway|path|steps|cycleway|bridleway|track|corridor|elevator|construction|proposed)$\"];")
        TEXT(");out geom;"),
        FMath::RoundToInt(SearchRadiusMeters), OriginLatitude, OriginLongitude,
        FMath::RoundToInt(SearchRadiusMeters), OriginLatitude, OriginLongitude);

    const FString Body = TEXT("data=") + FGenericPlatformHttp::UrlEncode(Query);

    // Independent Overpass instances; the first is usually the most responsive.
    static const TCHAR* const OverpassEndpoints[] = {
        TEXT("https://overpass.openstreetmap.fr/api/interpreter"),
        TEXT("https://overpass-api.de/api/interpreter"),
        TEXT("https://overpass.openstreetmap.fr/api/interpreter"),
    };
    const int32 EndpointIndex = FMath::Clamp(OverpassAttempt, 0, UE_ARRAY_COUNT(OverpassEndpoints) - 1);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(OverpassEndpoints[EndpointIndex]);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("Honeycomb/1.0 (Unreal)"));
    Request->SetContentAsString(Body);
    Request->SetTimeout(120.0f);
    const uint32 CapturedRequestId = RequestId;
    Request->OnProcessRequestComplete().BindWeakLambda(
        this, [this, CapturedRequestId](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk)
        {
            HandleOverpassResponse(CapturedRequestId, MoveTemp(Req), MoveTemp(Resp), bOk);
        });

    PendingRequest = Request;
    UE_LOG(LogTemp, Log, TEXT("[Honeycomb] Requesting OSM data (radius %d m, attempt %d, %s)..."),
        FMath::RoundToInt(SearchRadiusMeters), OverpassAttempt + 1, OverpassEndpoints[EndpointIndex]);
    Request->ProcessRequest();
}

void UGeoStreetSubsystem::HandleOverpassResponse(
    uint32 RequestId,
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
    TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
    bool bConnectedOk)
{
    using namespace GeoStreetPrivate;

    if (RequestId != BuildRequestId)
    {
        return;
    }

    const int32 Code = Response.IsValid() ? Response->GetResponseCode() : -1;
    UE_LOG(LogTemp, Log, TEXT("[Honeycomb] Overpass HTTP: connected=%d code=%d attempt=%d"),
        bConnectedOk ? 1 : 0, Code, OverpassAttempt + 1);

    if (!bConnectedOk || !Response.IsValid())
    {
        RetryNextOverpassOrFail(RequestId, TEXT("Overpass request did not return a response"));
        return;
    }

    if (Code != 200)
    {
        FString Content = Response->GetContentAsString();
        if (Content.Len() > 300)
        {
            Content = Content.Left(300);
        }
        UE_LOG(LogTemp, Warning, TEXT("[Honeycomb] Overpass body: %s"), *Content);
        RetryNextOverpassOrFail(RequestId, FString::Printf(TEXT("Overpass responded with HTTP %d"), Code));
        return;
    }

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader =
        TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        RetryNextOverpassOrFail(RequestId, TEXT("Overpass response was not valid JSON"));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* Elements = nullptr;
    if (!Root->TryGetArrayField(TEXT("elements"), Elements) || !Elements)
    {
        RetryNextOverpassOrFail(RequestId, TEXT("Overpass response had no elements"));
        return;
    }

    TArray<FGeoBuilding> Buildings;
    TArray<FGeoRoad> Roads;

    for (const TSharedPtr<FJsonValue>& ElementValue : *Elements)
    {
        const TSharedPtr<FJsonObject>* ElementPtr = nullptr;
        if (!ElementValue.IsValid() || !ElementValue->TryGetObject(ElementPtr) || !ElementPtr)
        {
            continue;
        }
        const TSharedPtr<FJsonObject>& Element = *ElementPtr;

        FString Type;
        if (!Element->TryGetStringField(TEXT("type"), Type) || Type != TEXT("way"))
        {
            continue;
        }

        const TArray<TSharedPtr<FJsonValue>>* Geometry = nullptr;
        if (!Element->TryGetArrayField(TEXT("geometry"), Geometry) || !Geometry || Geometry->Num() < 3)
        {
            continue;
        }

        TArray<FVector2D> Points;
        Points.Reserve(Geometry->Num());
        for (const TSharedPtr<FJsonValue>& PointValue : *Geometry)
        {
            const TSharedPtr<FJsonObject>* PointPtr = nullptr;
            if (!PointValue.IsValid() || !PointValue->TryGetObject(PointPtr) || !PointPtr)
            {
                continue;
            }
            double PLat = 0.0;
            double PLon = 0.0;
            if ((*PointPtr)->TryGetNumberField(TEXT("lat"), PLat) &&
                (*PointPtr)->TryGetNumberField(TEXT("lon"), PLon))
            {
                Points.Add(ProjectToLocalCm(PLat, PLon, OriginLatitude, OriginLongitude));
            }
        }

        if (Points.Num() < 2)
        {
            continue;
        }

        const TSharedPtr<FJsonObject>* TagsPtr = nullptr;
        const TSharedPtr<FJsonObject>* Tags = nullptr;
        if (Element->TryGetObjectField(TEXT("tags"), TagsPtr) && TagsPtr)
        {
            Tags = TagsPtr;
        }

        FString BuildingTag;
        FString HighwayTag;
        if (Tags)
        {
            (*Tags)->TryGetStringField(TEXT("building"), BuildingTag);
            (*Tags)->TryGetStringField(TEXT("highway"), HighwayTag);
        }

        if (!BuildingTag.IsEmpty() && Points.Num() >= 3)
        {
            float HeightCm = 800.0f;
            if (Tags)
            {
                FString HeightStr;
                FString LevelsStr;
                if ((*Tags)->TryGetStringField(TEXT("height"), HeightStr) && !HeightStr.IsEmpty())
                {
                    HeightCm = ParseLeadingFloat(HeightStr, 8.0f) * 100.0f;
                }
                else if ((*Tags)->TryGetStringField(TEXT("building:levels"), LevelsStr) && !LevelsStr.IsEmpty())
                {
                    HeightCm = ParseLeadingFloat(LevelsStr, 2.5f) * 320.0f;
                }
            }

            FGeoBuilding Building;
            Building.Footprint = Points;
            Building.Height = FMath::Clamp(HeightCm, 300.0f, 20000.0f);
            Building.Kind = BuildingTag;
            Buildings.Add(MoveTemp(Building));
        }
        else if (!HighwayTag.IsEmpty())
        {
            FGeoRoad Road;
            Road.Centerline = Points;
            Road.Width = RoadWidthCm(HighwayTag);
            Road.Kind = HighwayTag;
            Roads.Add(MoveTemp(Road));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[Honeycomb] OSM parsed: %d buildings, %d roads"),
        Buildings.Num(), Roads.Num());

    if (Buildings.Num() == 0 && Roads.Num() == 0)
    {
        RetryNextOverpassOrFail(RequestId, TEXT("Overpass returned no buildings or roads for this location"));
        return;
    }

    SpawnStreet(Buildings, Roads);
}

void UGeoStreetSubsystem::RetryNextOverpassOrFail(uint32 RequestId, const FString& Reason)
{
    constexpr int32 MaxOverpassAttempts = 3;
    if (RequestId == BuildRequestId && OverpassAttempt < MaxOverpassAttempts - 1)
    {
        ++OverpassAttempt;
        UE_LOG(LogTemp, Warning, TEXT("[Honeycomb] Overpass attempt %d failed (%s); retrying (attempt %d)"),
            OverpassAttempt, *Reason, OverpassAttempt + 1);
        RequestOverpass(RequestId);
        return;
    }

    Fail(Reason);
}

void UGeoStreetSubsystem::SpawnStreet(
    const TArray<FGeoBuilding>& Buildings,
    const TArray<FGeoRoad>& Roads)
{
    UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    if (!World)
    {
        Fail(TEXT("No world available to spawn the street"));
        return;
    }

    if (StreetActor)
    {
        StreetActor->Destroy();
        StreetActor = nullptr;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    StreetActor = World->SpawnActor<AProceduralStreet>(
        AProceduralStreet::StaticClass(), FTransform::Identity, Params);

    if (!StreetActor)
    {
        Fail(TEXT("Failed to spawn AProceduralStreet"));
        return;
    }

    StreetActor->SetActorLabel(LastLocation.City.IsEmpty() ? TEXT("Street") : LastLocation.City);
    StreetActor->BuildStreet(Buildings, Roads);

    bBusy = false;
    OnStreetBuilt.Broadcast(StreetActor->GetBuildingCount());

    UE_LOG(LogTemp, Log, TEXT("[Honeycomb] Street ready: %d buildings, %d roads, %d triangles"),
        StreetActor->GetBuildingCount(), StreetActor->GetRoadCount(), StreetActor->GetTriangleCount());
}

void UGeoStreetSubsystem::Fail(const FString& Reason)
{
    bBusy = false;
    UE_LOG(LogTemp, Warning, TEXT("[Honeycomb] Street build failed: %s"), *Reason);
    OnStreetFailed.Broadcast(Reason);
}
