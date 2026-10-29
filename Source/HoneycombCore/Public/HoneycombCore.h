#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "UObject/Object.h"

#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <queue>
#include <stack>
#include <string>
#include <string_view>
#include <algorithm>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <optional>
#include <variant>
#include <tuple>
#include <any>
#include <concepts>
#include <type_traits>
#include <cassert>
#include <cstring>
#include <cmath>
#include <ctime>
#include <ranges>
#include <span>
#include <bit>
#include <numbers>

namespace Core
{
    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T>
    using WeakRef = std::weak_ptr<T>;

    template<typename T>
    using Vec = std::vector<T>;

    template<typename K, typename V>
    using Map = std::unordered_map<K, V>;

    template<typename K, typename V>
    using OrderedMap = std::map<K, V>;

    template<typename T>
    using Set = std::set<T>;

    template<typename T>
    using List = std::list<T>;

    template<typename T>
    using Queue = std::queue<T>;

    template<typename T>
    using Stack = std::stack<T>;

    using String = std::string;
    using WString = std::wstring;
    using StringView = std::string_view;
    using WStringView = std::wstring_view;

    template<typename T>
    using Opt = std::optional<T>;

    template<typename T>
    using Span = std::span<T>;

    template<typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    template<typename T>
    concept Iterable = requires(T t) {
        t.begin();
        t.end();
    };

    template<typename T>
    concept Hashable = requires(T t) {
        std::hash<T>{}(t);
    };

    template<typename... Args>
    class Signal
    {
        public:
            using Callback = std::function<void(Args...)>;

            void Connect(Callback&& Callback)
            {
                Callbacks.push_back(std::move(Callback));
            }

            void Disconnect(const Callback& Target)
            {
                auto It = std::find_if(Callbacks.begin(), Callbacks.end(),
                    [&Target](const Callback& Cb) { return Cb.target_type() == Target.target_type(); });
                if (It != Callbacks.end())
                {
                    Callbacks.erase(It);
                }
            }

            void Emit(Args... args)
            {
                for (const auto& Cb : Callbacks)
                {
                    if (Cb)
                    {
                        Cb(args...);
                    }
                }
            }

            void Clear()
            {
                Callbacks.clear();
            }

            bool IsEmpty() const
            {
                return Callbacks.empty();
            }

        private:
            std::vector<Callback> Callbacks;
    };

    using DateTime = std::chrono::system_clock::time_point;

    DateTime Now();
    DateTime FromTimeT(time_t Time);
    time_t ToTimeT(const DateTime& Time);
    std::string DateTimeToString(const DateTime& Time);
    std::string DateTimeToISO(const DateTime& Time);
    double SecondsSince(const DateTime& Start);
    double MillisecondsSince(const DateTime& Start);

    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    namespace Internal
    {
        DECLARE_LOG_CATEGORY_EXTERN(LogHoneycombCore, Log, All);

        inline std::string FormatImpl(std::string Fmt)
        {
            return Fmt;
        }

        inline void ReplaceAll(std::string& Str, const std::string& From, const std::string& To)
        {
            size_t Pos = 0;
            while ((Pos = Str.find(From, Pos)) != std::string::npos)
            {
                Str.replace(Pos, From.length(), To);
                Pos += To.length();
            }
        }

        template<typename T, typename... Args>
        std::string FormatImpl(std::string Fmt, const T& Value, Args&&... args)
        {
            std::ostringstream OS;
            OS << Value;
            ReplaceAll(Fmt, "{}", OS.str());
            return FormatImpl(std::move(Fmt), std::forward<Args>(args)...);
        }

        inline FString ToFString(const std::string& Str)
        {
            return FString(ANSI_TO_TCHAR(Str.c_str()));
        }
    }

    class Logger
    {
        public:
            static Logger& Instance()
            {
                static Logger LocalInstance;
                return LocalInstance;
            }

            template<typename... Args>
            void Debug(const std::string& Fmt, Args&&... args)
            {
                Log(LogLevel::Debug, Internal::FormatImpl(Fmt, std::forward<Args>(args)...));
            }

            template<typename... Args>
            void Info(const std::string& Fmt, Args&&... args)
            {
                Log(LogLevel::Info, Internal::FormatImpl(Fmt, std::forward<Args>(args)...));
            }

            template<typename... Args>
            void Warning(const std::string& Fmt, Args&&... args)
            {
                Log(LogLevel::Warning, Internal::FormatImpl(Fmt, std::forward<Args>(args)...));
            }

            template<typename... Args>
            void Error(const std::string& Fmt, Args&&... args)
            {
                Log(LogLevel::Error, Internal::FormatImpl(Fmt, std::forward<Args>(args)...));
            }

            template<typename... Args>
            void Fatal(const std::string& Fmt, Args&&... args)
            {
                Log(LogLevel::Fatal, Internal::FormatImpl(Fmt, std::forward<Args>(args)...));
            }

        private:
            void Log(LogLevel Level, const std::string& Message)
            {
                std::lock_guard<std::mutex> Lock(Mutex);
                FString Msg = Internal::ToFString(Message);

                switch (Level)
                {
                    case LogLevel::Debug:   UE_LOG(Internal::LogHoneycombCore, Verbose, TEXT("%s"), *Msg); break;
                    case LogLevel::Info:    UE_LOG(Internal::LogHoneycombCore, Log, TEXT("%s"), *Msg); break;
                    case LogLevel::Warning: UE_LOG(Internal::LogHoneycombCore, Warning, TEXT("%s"), *Msg); break;
                    case LogLevel::Error:   UE_LOG(Internal::LogHoneycombCore, Error, TEXT("%s"), *Msg); break;
                    case LogLevel::Fatal:   UE_LOG(Internal::LogHoneycombCore, Fatal, TEXT("%s"), *Msg); break;
                }

                if (Level == LogLevel::Fatal)
                {
                    TCHAR Buffer[1024] = {};
                    FCString::Sprintf(Buffer, TEXT("%s\n"), *Msg);
                    checkf(false, TEXT("%s"), Buffer);
                }
            }

            Logger() = default;
            std::mutex Mutex;
    };

    #define LOG_DEBUG(...) Core::Logger::Instance().Debug(__VA_ARGS__)
    #define LOG_INFO(...) Core::Logger::Instance().Info(__VA_ARGS__)
    #define LOG_WARNING(...) Core::Logger::Instance().Warning(__VA_ARGS__)
    #define LOG_ERROR(...) Core::Logger::Instance().Error(__VA_ARGS__)
    #define LOG_FATAL(...) Core::Logger::Instance().Fatal(__VA_ARGS__)

    template<typename T, typename... Args>
    Scope<T> MakeScope(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    Ref<T> MakeRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<Arithmetic T>
    T Clamp(T Value, T Min, T Max)
    {
        return std::max(Min, std::min(Value, Max));
    }

    template<Arithmetic T>
    T Lerp(T A, T B, double TFactor)
    {
        return A + (B - A) * TFactor;
    }

    template<Iterable Container>
    auto Take(Container&& C, size_t N)
    {
        return C | std::views::take(N);
    }

    template<Iterable Container>
    auto Drop(Container&& C, size_t N)
    {
        return C | std::views::drop(N);
    }

    template<Iterable Container>
    auto Filter(Container&& C, auto Pred)
    {
        return C | std::views::filter(Pred);
    }

    template<Iterable Container>
    auto Transform(Container&& C, auto Fn)
    {
        return C | std::views::transform(Fn);
    }

    std::string ToLower(const std::string& Str);
    std::string ToUpper(const std::string& Str);
    std::string Trim(const std::string& Str);
    std::string Replace(const std::string& Str, const std::string& From, const std::string& To);

    std::vector<std::string> Split(const std::string& Str, char Delimiter);
    std::vector<std::string> Split(const std::string& Str, const std::string& Delimiter);
    std::string Join(const std::vector<std::string>& Tokens, const std::string& Delimiter);

    bool StartsWith(const std::string& Str, const std::string& Prefix);
    bool EndsWith(const std::string& Str, const std::string& Suffix);

    bool FileExists(const std::string& Path);
    bool DirectoryExists(const std::string& Path);
    bool CreateDirectory(const std::string& Path);
    bool DeleteFile(const std::string& Path);
    bool DeleteDirectory(const std::string& Path);

    std::string GetFileName(const std::string& Path);
    std::string GetDirectory(const std::string& Path);
    std::string GetExtension(const std::string& Path);
    std::string ChangeExtension(const std::string& Path, const std::string& NewExtension);

    std::string GetCurrentDirectory();
    bool SetCurrentDirectory(const std::string& Path);

    std::vector<std::string> GetFiles(const std::string& Directory, const std::string& Extension);
    std::vector<std::string> GetDirectories(const std::string& Directory);

    std::string ReadTextFile(const std::string& Path);
    bool WriteTextFile(const std::string& Path, const std::string& Content);

    std::vector<uint8_t> ReadBinaryFile(const std::string& Path);
    bool WriteBinaryFile(const std::string& Path, const std::vector<uint8_t>& Data);

    void* Malloc(size_t Size);
    void Free(void* Ptr);
    void* Calloc(size_t Count, size_t Size);
    void* Realloc(void* Ptr, size_t NewSize);

    int GetProcessorCount();
    std::string GetPlatformName();
    std::string GetCompilerName();
    int GetCompilerVersion();
    std::string GetCppStandard();

    uint64_t GenerateId();
    std::string GenerateUUID();

    float Lerp(float A, float B, float T);
    double Lerp(double A, double B, double T);

    float Clamp(float Value, float Min, float Max);
    double Clamp(double Value, double Min, double Max);

    float SmoothStep(float Edge0, float Edge1, float X);
    float MapRange(float Value, float InMin, float InMax, float OutMin, float OutMax);

    int RandomInt(int Min, int Max);
    float RandomFloat(float Min, float Max);
    double RandomDouble(double Min, double Max);

    constexpr int CoreVersion = 1'00'00;

    inline std::string GetCoreVersionString()
    {
        return "Core v1.0.0 (UE)";
    }
}

inline FString ToFString(const std::string& Str)
{
    return FString(ANSI_TO_TCHAR(Str.c_str()));
}

inline std::string ToStdString(const FString& Str)
{
    return std::string(TCHAR_TO_ANSI(*Str));
}