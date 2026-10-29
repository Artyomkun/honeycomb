#include "HoneycombCore.h"

#include "Misc/DateTime.h"
#include "HAL/PlatformMisc.h"
#include "UObject/UObjectGlobals.h"

namespace Core::Internal
{
    DEFINE_LOG_CATEGORY(LogHoneycombCore);
}

namespace Core
{

    int GetCoreVersion() { return CoreVersion; }

    void* Malloc(size_t Size)
    {
        return FMemory::Malloc(Size);
    }

    void Free(void* Ptr)
    {
        FMemory::Free(Ptr);
    }

    void* Calloc(size_t Count, size_t Size)
    {
        void* Ptr = FMemory::Malloc(Count * Size);
        if (Ptr)
        {
            FMemory::Memzero(Ptr, Count * Size);
        }
        return Ptr;
    }

    void* Realloc(void* Ptr, size_t NewSize)
    {
        return FMemory::Realloc(Ptr, NewSize);
    }

    DateTime Now()
    {
        return std::chrono::system_clock::now();
    }

    DateTime FromTimeT(time_t Time)
    {
        return std::chrono::system_clock::from_time_t(Time);
    }

    time_t ToTimeT(const DateTime& Time)
    {
        return std::chrono::system_clock::to_time_t(Time);
    }

    std::string DateTimeToString(const DateTime& Time)
    {
        return DateTimeToISO(Time);
    }

    std::string DateTimeToISO(const DateTime& Time)
    {
        auto TimeT = std::chrono::system_clock::to_time_t(Time);
        std::tm TM;

        #ifdef _MSC_VER
            gmtime_s(&TM, &TimeT);
        #else
            gmtime_r(&TimeT, &TM);
        #endif

        std::ostringstream Stream;
        Stream << std::put_time(&TM, "%Y-%m-%dT%H:%M:%SZ");
        return Stream.str();
    }

    double SecondsSince(const DateTime& Start)
    {
        using namespace std::chrono;
        return duration<double>(Now() - Start).count();
    }

    double MillisecondsSince(const DateTime& Start)
    {
        using namespace std::chrono;
        return duration<double, std::milli>(Now() - Start).count();
    }

    std::string ToLower(const std::string& Str)
    {
        std::string Result = Str;
        std::transform(Result.begin(), Result.end(), Result.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return Result;
    }

    std::string ToUpper(const std::string& Str)
    {
        std::string Result = Str;
        std::transform(Result.begin(), Result.end(), Result.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return Result;
    }

    std::string Trim(const std::string& Str)
    {
        auto Start = Str.find_first_not_of(" \t\n\r");
        if (Start == std::string::npos) return "";

        auto End = Str.find_last_not_of(" \t\n\r");
        return Str.substr(Start, End - Start + 1);
    }

    std::string Replace(const std::string& Str, const std::string& From, const std::string& To)
    {
        if (From.empty()) return Str;

        std::string Result = Str;
        size_t Pos = 0;
        while ((Pos = Result.find(From, Pos)) != std::string::npos)
        {
            Result.replace(Pos, From.length(), To);
            Pos += To.length();
        }
        return Result;
    }

    std::vector<std::string> Split(const std::string& Str, char Delimiter)
    {
        std::vector<std::string> Tokens;
        std::stringstream SS(Str);
        std::string Token;

        while (std::getline(SS, Token, Delimiter))
        {
            if (!Token.empty())
            {
                Tokens.push_back(Token);
            }
        }

        return Tokens;
    }

    std::vector<std::string> Split(const std::string& Str, const std::string& Delimiter)
    {
        std::vector<std::string> Tokens;
        size_t Start = 0;
        size_t End = 0;

        while ((End = Str.find(Delimiter, Start)) != std::string::npos)
        {
            std::string Token = Str.substr(Start, End - Start);
            if (!Token.empty())
            {
                Tokens.push_back(Token);
            }
            Start = End + Delimiter.length();
        }

        std::string LastToken = Str.substr(Start);
        if (!LastToken.empty())
        {
            Tokens.push_back(LastToken);
        }

        return Tokens;
    }

    std::string Join(const std::vector<std::string>& Tokens, const std::string& Delimiter)
    {
        if (Tokens.empty()) return "";

        std::string Result = Tokens[0];
        for (size_t i = 1; i < Tokens.size(); ++i)
        {
            Result += Delimiter + Tokens[i];
        }
        return Result;
    }

    bool StartsWith(const std::string& Str, const std::string& Prefix)
    {
        return Str.rfind(Prefix, 0) == 0;
    }

    bool EndsWith(const std::string& Str, const std::string& Suffix)
    {
        if (Suffix.length() > Str.length()) return false;
        return Str.compare(Str.length() - Suffix.length(), Suffix.length(), Suffix) == 0;
    }

    #define HC_TRY_FS(op) \
        try { return (op); } \
        catch (const std::filesystem::filesystem_error& e) { LOG_ERROR("{}", e.what()); } \
        catch (const std::exception& e) { LOG_ERROR("Unknown error: {}", e.what()); } \
        return false;

    bool FileExists(const std::string& Path)
    {
        HC_TRY_FS(std::filesystem::exists(Path) && std::filesystem::is_regular_file(Path));
    }

    bool DirectoryExists(const std::string& Path)
    {
        HC_TRY_FS(std::filesystem::exists(Path) && std::filesystem::is_directory(Path));
    }

    bool CreateDirectory(const std::string& Path)
    {
        HC_TRY_FS(std::filesystem::create_directories(Path));
    }

    bool DeleteFile(const std::string& Path)
    {
        HC_TRY_FS(std::filesystem::remove(Path));
    }

    bool DeleteDirectory(const std::string& Path)
    {
        HC_TRY_FS(std::filesystem::remove_all(Path) > 0);
    }

    #undef HC_TRY_FS

    std::string GetFileName(const std::string& Path)
    {
        try { return std::filesystem::path(Path).filename().string(); }
        catch (...) { return ""; }
    }

    std::string GetDirectory(const std::string& Path)
    {
        try { return std::filesystem::path(Path).parent_path().string(); }
        catch (...) { return ""; }
    }

    std::string GetExtension(const std::string& Path)
    {
        try { return std::filesystem::path(Path).extension().string(); }
        catch (...) { return ""; }
    }

    std::string ChangeExtension(const std::string& Path, const std::string& NewExtension)
    {
        try { return std::filesystem::path(Path).replace_extension(NewExtension).string(); }
        catch (...) { return Path; }
    }

    std::string GetCurrentDirectory()
    {
        try { return std::filesystem::current_path().string(); }
        catch (...) { return ""; }
    }

    bool SetCurrentDirectory(const std::string& Path)
    {
        try { std::filesystem::current_path(Path); return true; }
        catch (...) { return false; }
    }

    std::vector<std::string> GetFiles(const std::string& Directory, const std::string& Extension)
    {
        std::vector<std::string> Files;
        try
        {
            for (const auto& Entry : std::filesystem::directory_iterator(Directory))
            {
                if (Entry.is_regular_file())
                {
                    const auto& Path = Entry.path();
                    if (Extension.empty() || Path.extension() == Extension)
                    {
                        Files.push_back(Path.string());
                    }
                }
            }
        }
        catch (...) { }
        return Files;
    }

    std::vector<std::string> GetDirectories(const std::string& Directory)
    {
        std::vector<std::string> Directories;
        try
        {
            for (const auto& Entry : std::filesystem::directory_iterator(Directory))
            {
                if (Entry.is_directory())
                {
                    Directories.push_back(Entry.path().string());
                }
            }
        }
        catch (...) { }
        return Directories;
    }

    std::string ReadTextFile(const std::string& Path)
    {
        try
        {
            std::ifstream File(Path);
            if (!File.is_open())
            {
                LOG_WARNING("ReadTextFile: Cannot open file: {}", Path);
                return "";
            }

            std::stringstream Buffer;
            Buffer << File.rdbuf();
            return Buffer.str();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("ReadTextFile: {}", e.what());
            return "";
        }
    }

    bool WriteTextFile(const std::string& Path, const std::string& Content)
    {
        try
        {
            std::ofstream File(Path);
            if (!File.is_open())
            {
                LOG_WARNING("WriteTextFile: Cannot open file: {}", Path);
                return false;
            }

            File << Content;
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("WriteTextFile: {}", e.what());
            return false;
        }
    }

    std::vector<uint8_t> ReadBinaryFile(const std::string& Path)
    {
        try
        {
            std::ifstream File(Path, std::ios::binary | std::ios::ate);
            if (!File.is_open())
            {
                LOG_WARNING("ReadBinaryFile: Cannot open file: {}", Path);
                return {};
            }

            std::streamsize Size = File.tellg();
            File.seekg(0, std::ios::beg);

            std::vector<uint8_t> Data(static_cast<size_t>(Size));
            if (File.read(reinterpret_cast<char*>(Data.data()), Size))
            {
                return Data;
            }

            LOG_WARNING("ReadBinaryFile: Failed to read data from: {}", Path);
            return {};
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("ReadBinaryFile: {}", e.what());
            return {};
        }
    }

    bool WriteBinaryFile(const std::string& Path, const std::vector<uint8_t>& Data)
    {
        try
        {
            std::ofstream File(Path, std::ios::binary);
            if (!File.is_open())
            {
                LOG_WARNING("WriteBinaryFile: Cannot open file: {}", Path);
                return false;
            }

            File.write(reinterpret_cast<const char*>(Data.data()), static_cast<std::streamsize>(Data.size()));
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("WriteBinaryFile: {}", e.what());
            return false;
        }
    }

    int GetProcessorCount()
    {
        return FPlatformMisc::NumberOfCores();
    }

    std::string GetPlatformName()
    {
        return std::string(TCHAR_TO_UTF8(*FPlatformMisc::GetOSVersion()));
    }

    std::string GetCompilerName()
    {
        #ifdef __clang__
            return "Clang";
        #elif defined(__GNUC__)
            return "GCC";
        #elif defined(_MSC_VER)
            return "MSVC";
        #else
            return "Unknown";
        #endif
    }

    int GetCompilerVersion()
    {
        #ifdef __clang__
            return __clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__;
        #elif defined(__GNUC__)
            return __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__;
        #elif defined(_MSC_VER)
            return _MSC_VER;
        #else
            return 0;
        #endif
    }

    std::string GetCppStandard()
    {
        #if defined(__cplusplus) && __cplusplus >= 202302L
            return "C++23";
        #elif defined(__cplusplus) && __cplusplus >= 202002L
            return "C++20";
        #elif defined(__cplusplus) && __cplusplus >= 201703L
            return "C++17";
        #else
            return "C++14";
        #endif
    }

    uint64_t GenerateId()
    {
        static std::atomic<uint64_t> Counter{0};
        return ++Counter;
    }

    std::string GenerateUUID()
    {
        FGuid Guid = FGuid::NewGuid();
        return std::string(TCHAR_TO_UTF8(*Guid.ToString()));
    }

    float Lerp(float A, float B, float T)
    {
        return A + (B - A) * T;
    }

    double Lerp(double A, double B, double T)
    {
        return A + (B - A) * T;
    }

    float Clamp(float Value, float Min, float Max)
    {
        return std::max(Min, std::min(Value, Max));
    }

    double Clamp(double Value, double Min, double Max)
    {
        return std::max(Min, std::min(Value, Max));
    }

    float SmoothStep(float Edge0, float Edge1, float X)
    {
        float T = Clamp((X - Edge0) / (Edge1 - Edge0), 0.0f, 1.0f);
        return T * T * (3.0f - 2.0f * T);
    }

    float MapRange(float Value, float InMin, float InMax, float OutMin, float OutMax)
    {
        float T = (Value - InMin) / (InMax - InMin);
        return OutMin + (OutMax - OutMin) * T;
    }

    int RandomInt(int Min, int Max)
    {
        static std::random_device RD;
        static std::mt19937 Gen(RD());
        std::uniform_int_distribution<int> Dist(Min, Max);
        return Dist(Gen);
    }

    float RandomFloat(float Min, float Max)
    {
        static std::random_device RD;
        static std::mt19937 Gen(RD());
        std::uniform_real_distribution<float> Dist(Min, Max);
        return Dist(Gen);
    }

    double RandomDouble(double Min, double Max)
    {
        static std::random_device RD;
        static std::mt19937_64 Gen(RD());
        std::uniform_real_distribution<double> Dist(Min, Max);
        return Dist(Gen);
    }
}

static class CoreInitializer
{
public:
    CoreInitializer()
    {
        LOG_INFO("Core framework v{} initialized on {}",
            Core::CoreVersion,
            Core::GetPlatformName());
    }
} CoreInit;

#include "Modules/ModuleManager.h"
IMPLEMENT_MODULE(FDefaultModuleImpl, HoneycombCore);