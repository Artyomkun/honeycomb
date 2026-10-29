#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <cctype>
#include <span>
#include <memory>
#include <optional>
#include <expected>
#include <variant>
#include <tuple>
#include <functional>
#include <chrono>
#include <format>
#include <print>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <concepts>
#include <type_traits>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <semaphore>
#include <latch>
#include <barrier>
#include <random>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cassert>
#include <cstring>
#include <csignal>

#ifdef _WIN32
    using socket_t = std::uint64_t;
#else
    using socket_t = int;
#endif
    constexpr socket_t INVALID_SOCKET_VALUE = static_cast<socket_t>(-1);
    constexpr int SOCKET_ERROR_VALUE = -1;

namespace Honeycomb::Core
{
    using i8  = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using f32 = float;
    using f64 = double;
    using usize = std::size_t;

    using String      = std::string;
    using StringView  = std::string_view;
    using WString     = std::wstring;
    using WStringView = std::wstring_view;

    template<typename T>
    using Vec = std::vector<T>;

    template<typename K, typename V>
    using Map = std::unordered_map<K, V>;

    template<typename K, typename V>
    using OrderedMap = std::map<K, V>;

    template<typename T>
    using Set = std::set<T>;

    template<typename T>
    using Opt = std::optional<T>;

    template<typename T, typename E>
    using Result = std::expected<T, E>;

    template<typename... Ts>
    using Overload = std::variant<Ts...>;

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T>
    using WeakRef = std::weak_ptr<T>;

    template<typename T>
    using Span = std::span<T>;

    using Thread = std::thread;

    enum class ErrorCode: u32
    {
        None = 0,
        SocketCreateFailed,
        SocketBindFailed,
        SocketListenFailed,
        SocketAcceptFailed,
        SocketConnectFailed,
        SocketSendFailed,
        SocketRecvFailed,
        SocketWouldBlock,
        SocketTimedOut,
        HttpParseError,
        HttpMalformedRequest,
        HttpMethodNotAllowed,
        HttpNotFound,
        HttpBadRequest,
        HttpInternalServerError,
        WebSocketHandshakeFailed,
        WebSocketFrameError,
        DatabaseConnectionFailed,
        DatabaseQueryFailed,
        DatabaseTransactionFailed,
        FeatureDisabled,
        QueryFailed,
        TransactionFailed,
        GameInvalidState,
        GamePlayerNotFound,
        GameSessionFull,
        GameAlreadyInSession,
        SerializationError,
        Unauthorized,
        RateLimited,
    };

    struct Error
    {
        ErrorCode code = ErrorCode::None;
        String message;
        String file;
        i32 line = 0;

        [[nodiscard]] constexpr bool has_error() const noexcept { return code != ErrorCode::None; }
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_error(); }

        [[nodiscard]] static Error make(ErrorCode c, String msg, String f = "", i32 l = 0)
        {
            return Error{ .code = c, .message = std::move(msg), .file = std::move(f), .line = l };
        }
    };

    #define HC_ERROR(code, msg) Honeycomb::Core::Error::make(code, msg, __FILE__, __LINE__)

    [[nodiscard]] inline String ToLower(std::string_view Str)
    {
        String Result;
        Result.reserve(Str.size());
        for (unsigned char c : Str)
        {
            Result.push_back(static_cast<char>(std::tolower(c)));
        }
        return Result;
    }

    [[nodiscard]] inline String Trim(std::string_view Str)
    {
        const auto Start = Str.find_first_not_of(" \t\r\n");
        if (Start == std::string_view::npos)
        {
            return String{};
        }
        const auto End = Str.find_last_not_of(" \t\r\n");
        return String(Str.substr(Start, End - Start + 1));
    }

    [[nodiscard]] inline Vec<String> Split(std::string_view Str, char Delimiter)
    {
        Vec<String> Tokens;
        std::string_view::size_type start = 0;
        while (true)
        {
            const auto pos = Str.find(Delimiter, start);
            if (pos == std::string_view::npos)
            {
                Tokens.push_back(String(Str.substr(start)));
                break;
            }
            Tokens.push_back(String(Str.substr(start, pos - start)));
            start = pos + 1;
        }
        return Tokens;
    }
}

namespace Honeycomb
{
    using namespace Honeycomb::Core;
}

#include "Logger.hpp"

namespace Honeycomb::Core
{
    [[nodiscard]] std::array<u8, 20> sha1(std::span<const u8> data);
}
