#include "Socket.hpp"
#include "Logger.hpp"

namespace Honeycomb::Network
{
    namespace
    {
        [[nodiscard]] Error MakeSocketError(ErrorCode Code, const TCHAR* Message)
        {
            return HC_ERROR(Code, std::string(TCHAR_TO_UTF8(Message)));
        }

        [[nodiscard]] bool TryMakeInternetAddr(std::string_view Address, u16 Port, TSharedPtr<FInternetAddr>& OutAddr)
        {
            ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
            if (!Subsystem)
            {
                return false;
            }

            OutAddr = Subsystem->CreateInternetAddr();

            if (Address.empty() || Address == "0.0.0.0")
            {
                OutAddr->SetAnyAddress();
            }
            else
            {
                FIPv4Address IpAddress;
                if (!FIPv4Address::Parse(ToFString(Address), IpAddress))
                {
                    return false;
                }
                OutAddr->SetIp(IpAddress.Value);
            }

            OutAddr->SetPort(static_cast<int32>(Port));
            return true;
        }

        [[nodiscard]] std::pair<String, u16> DescribeAddress(const FInternetAddr& Addr)
        {
            uint32 RawIp = 0;
            Addr.GetIp(RawIp);

            return {ToString(FIPv4Address(RawIp).ToString()), static_cast<u16>(Addr.GetPort())};
        }
    }

    Socket::Socket(FSocketRef Socket, ESocketType Type) noexcept: Socket_(std::move(Socket)), Type_(Type) { }

    Socket::~Socket() = default;

    Socket::Socket(Socket&& other) noexcept: Socket_(std::move(other.Socket_)), Type_(other.Type_) { }

    Socket& Socket::operator=(Socket&& other) noexcept
    {
        if (this != &other)
        {
            Socket_ = std::move(other.Socket_);
            Type_ = other.Type_;
        }
        return *this;
    }

    Result<Socket, Error> Socket::create_tcp()
    {
        ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        if (!Subsystem)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("Socket subsystem not available")));
        }

        FSocketRef NewSocket = MakeSocketRef(Subsystem->CreateSocket(NAME_Stream, TEXT("HoneycombTCP"), false), Subsystem);

        if (!NewSocket)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("Failed to create TCP socket")));
        }

        return Socket(std::move(NewSocket), ESocketType::SOCKTYPE_Streaming);
    }

    Result<Socket, Error> Socket::create_udp()
    {
        ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        if (!Subsystem)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("Socket subsystem not available")));
        }

        FSocketRef NewSocket = MakeSocketRef(Subsystem->CreateSocket(NAME_DGram, TEXT("HoneycombUDP"), false), Subsystem);

        if (!NewSocket)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("Failed to create UDP socket")));
        }

        return Socket(std::move(NewSocket), ESocketType::SOCKTYPE_Datagram);
    }

    Result<void, Error> Socket::bind(std::string_view address, u16 port)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketBindFailed, TEXT("Socket not valid")));
        }

        TSharedPtr<FInternetAddr> BindAddr;
        if (!TryMakeInternetAddr(address, port, BindAddr) || !BindAddr)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketBindFailed, TEXT("Invalid address format")));
        }

        if (!Socket_->Bind(*BindAddr))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketBindFailed, TEXT("Bind failed")));
        }

        return {};
    }

    Result<void, Error> Socket::listen(i32 backlog)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketListenFailed, TEXT("Socket not valid")));
        }

        if (!Socket_->Listen(static_cast<int32>(backlog)))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketListenFailed, TEXT("Listen failed")));
        }

        return {};
    }

    std::expected<Socket, Error> Socket::accept()
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketAcceptFailed, TEXT("Socket not valid")));
        }

        FSocket* AcceptedRaw = Socket_->Accept(TEXT("HoneycombAccepted"));

        if (AcceptedRaw == nullptr)
        {
            bool bHasPending = false;
            Socket_->HasPendingConnection(bHasPending);
            if (!bHasPending)
            {
                return std::unexpected(MakeSocketError(ErrorCode::SocketWouldBlock, TEXT("No pending connection")));
            }

            return std::unexpected(MakeSocketError(ErrorCode::SocketAcceptFailed, TEXT("Accept failed")));
        }

        return Socket(MakeSocketRef(AcceptedRaw), ESocketType::SOCKTYPE_Streaming);
    }

    Result<i32, Error> Socket::send(std::span<const u8> data)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketSendFailed, TEXT("Socket not valid")));
        }

        int32 BytesSent = 0;
        if (!Socket_->Send(data.data(), static_cast<int32>(data.size()), BytesSent))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketSendFailed, TEXT("Send failed")));
        }

        return BytesSent;
    }

    Result<i32, Error> Socket::send_to(std::span<const u8> data, std::string_view address, u16 port)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketSendFailed, TEXT("Socket not valid")));
        }

        TSharedPtr<FInternetAddr> DestinationAddr;
        if (!TryMakeInternetAddr(address, port, DestinationAddr) || !DestinationAddr)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketSendFailed, TEXT("Invalid address format")));
        }

        int32 BytesSent = 0;
        if (!Socket_->SendTo(data.data(), static_cast<int32>(data.size()), BytesSent, *DestinationAddr))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketSendFailed, TEXT("SendTo failed")));
        }

        return BytesSent;
    }

    Result<Vec<u8>, Error> Socket::recv(i32 flags)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketRecvFailed, TEXT("Socket not valid")));
        }

        uint32 BufferSize = 4096;
        Socket_->HasPendingData(BufferSize);

        if (BufferSize == 0)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketWouldBlock, TEXT("No pending data")));
        }

        Vec<u8> Buffer(BufferSize);
        int32 BytesRead = 0;
        if (!Socket_->Recv(Buffer.data(), static_cast<int32>(Buffer.size()), BytesRead, static_cast<ESocketReceiveFlags::Type>(flags)))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketRecvFailed, TEXT("Recv failed")));
        }

        Buffer.resize(static_cast<usize>(BytesRead));
        return Buffer;
    }

    Result<std::tuple<Vec<u8>, String, u16>, Error> Socket::recv_from()
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketRecvFailed, TEXT("Socket not valid")));
        }

        ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        if (!Subsystem)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketRecvFailed, TEXT("Socket subsystem not available")));
        }

        uint32 BufferSize = 4096;
        Socket_->HasPendingData(BufferSize);

        if (BufferSize == 0)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketWouldBlock, TEXT("No pending data")));
        }

        Vec<u8> Buffer(BufferSize);
        TSharedRef<FInternetAddr> SourceAddr = Subsystem->CreateInternetAddr();
        int32 BytesRead = 0;

        if (!Socket_->RecvFrom(Buffer.data(), static_cast<int32>(Buffer.size()), BytesRead, *SourceAddr))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketRecvFailed, TEXT("RecvFrom failed")));
        }

        Buffer.resize(static_cast<usize>(BytesRead));

        auto [Address, Port] = DescribeAddress(*SourceAddr);
        return std::make_tuple(std::move(Buffer), std::move(Address), Port);
    }

    Result<void, Error> Socket::set_nonblocking(bool enable)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("Socket not valid")));
        }

        if (!Socket_->SetNonBlocking(enable))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("SetNonBlocking failed")));
        }

        return {};
    }

    Result<void, Error> Socket::set_reuseaddr(bool enable)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("Socket not valid")));
        }

        if (!Socket_->SetReuseAddr(enable))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("SetReuseAddr failed")));
        }

        return {};
    }

    Result<void, Error> Socket::set_nodelay(bool enable)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("Socket not valid")));
        }

        if (!Socket_->SetNoDelay(enable))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("SetNoDelay failed")));
        }

        return {};
    }

    Result<void, Error> Socket::set_sndbuf(i32 size)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("Socket not valid")));
        }

        int32 NewSize = 0;
        if (!Socket_->SetSendBufferSize(static_cast<int32>(size), NewSize))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("SetSendBufferSize failed")));
        }

        return {};
    }

    Result<void, Error> Socket::set_rcvbuf(i32 size)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("Socket not valid")));
        }

        int32 NewSize = 0;
        if (!Socket_->SetReceiveBufferSize(static_cast<int32>(size), NewSize))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketCreateFailed, TEXT("SetReceiveBufferSize failed")));
        }

        return {};
    }

    Result<void, Error> Socket::close()
    {
        if (!Socket_)
        {
            return {};
        }

        Socket_->Close();
        return {};
    }

    bool Socket::is_valid() const noexcept
    {
        return Socket_.IsValid();
    }

    std::pair<String, u16> Socket::peer_addr() const
    {
        if (!Socket_)
        {
            return {"", 0};
        }

        ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        if (!Subsystem)
        {
            return {"", 0};
        }

        TSharedRef<FInternetAddr> PeerAddr = Subsystem->CreateInternetAddr();
        if (!Socket_->GetPeerAddress(*PeerAddr))
        {
            return {"", 0};
        }

        return DescribeAddress(*PeerAddr);
    }

    Result<void, Error> Socket::connect(std::string_view address, u16 port)
    {
        if (!Socket_)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketConnectFailed, TEXT("Socket not valid")));
        }

        TSharedPtr<FInternetAddr> ConnectAddr;
        if (!TryMakeInternetAddr(address, port, ConnectAddr) || !ConnectAddr)
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketConnectFailed, TEXT("Invalid address format")));
        }

        if (!Socket_->Connect(*ConnectAddr))
        {
            return std::unexpected(MakeSocketError(ErrorCode::SocketConnectFailed, TEXT("Connect failed")));
        }

        return {};
    }
}

