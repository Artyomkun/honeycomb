#pragma once
#include "Core.hpp"
#include "FSocketRef.hpp"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "SocketTypes.h"
#include "Interfaces/IPv4/IPv4Address.h"

namespace Honeycomb::Network
{
    using namespace Honeycomb::Core;

    [[nodiscard]] inline FString ToFString(std::string_view Str) { return FString(String(Str).c_str()); }
    [[nodiscard]] inline std::string ToStdString(const FString& Str) { return std::string(TCHAR_TO_UTF8(*Str)); }
    [[nodiscard]] inline std::string ToString(const FString& Str) { return ToStdString(Str); }

    class Socket
    {
        public:
            Socket() = default;
            ~Socket();

            Socket(const Socket&) = delete;
            Socket& operator=(const Socket&) = delete;
            Socket(Socket&& other) noexcept;
            Socket& operator=(Socket&& other) noexcept;

            [[nodiscard]] static Result<Socket, Error> create_tcp();
            [[nodiscard]] static Result<Socket, Error> create_udp();

            [[nodiscard]] Result<void, Error> bind(std::string_view address, u16 port);
            [[nodiscard]] Result<void, Error> listen(i32 backlog = 128);
            [[nodiscard]] std::expected<Socket, Error> accept();
            [[nodiscard]] Result<void, Error> connect(std::string_view address, u16 port);

            [[nodiscard]] Result<i32, Error> send(std::span<const u8> data);
            [[nodiscard]] Result<i32, Error> send_to(std::span<const u8> data, std::string_view address, u16 port);
            [[nodiscard]] Result<Vec<u8>, Error> recv(i32 flags = 0);
            [[nodiscard]] Result<std::tuple<Vec<u8>, String, u16>, Error> recv_from();

            [[nodiscard]] Result<void, Error> set_nonblocking(bool enable);
            [[nodiscard]] Result<void, Error> set_reuseaddr(bool enable);
            [[nodiscard]] Result<void, Error> set_nodelay(bool enable);
            [[nodiscard]] Result<void, Error> set_sndbuf(i32 size);
            [[nodiscard]] Result<void, Error> set_rcvbuf(i32 size);

            [[nodiscard]] Result<void, Error> close();

            [[nodiscard]] bool is_valid() const noexcept;
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            [[nodiscard]] std::pair<String, u16> peer_addr() const;
            [[nodiscard]] FSocket* GetNative() const noexcept { return Socket_.Get(); }

        private:
            FSocketRef Socket_;
            ESocketType Type_ = ESocketType::SOCKTYPE_Streaming;

            explicit Socket(FSocketRef Socket, ESocketType Type) noexcept;
    };

    using PlatformInit = std::monostate;
}
