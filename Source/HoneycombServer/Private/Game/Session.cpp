#include "Session.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "Logger.hpp"
#include "Sockets.h"
#include "SocketSubsystem.h"

namespace Honeycomb::Game
{
    Session::Session(SessionId id, Network::FSocketRef Conn, GameServer& server)
        : id_(id), Conn_(std::move(Conn)), server_(server)
    {
    }

    void Session::start()
    {
        auto [Address, Port] = addr();
        LOG_INFO("Session {} connected from {}:{}", id_, Address, Port);
    }

    void Session::shut_down()
    {
        if (!active_.exchange(false)) return;

        if (Conn_)
        {
            Conn_->Close();
        }

        if (on_disconnect_cb_)
        {
            on_disconnect_cb_(shared_from_this());
        }
    }

    Result<void, Error> Session::send(Vec<u8> data)
    {
        if (!active_) return std::unexpected(HC_ERROR(ErrorCode::GameInvalidState, "Session inactive"));
        if (!Conn_)
        {
            return std::unexpected(HC_ERROR(ErrorCode::SocketSendFailed, "Socket invalid"));
        }

        int32 BytesSent = 0;
        if (!Conn_->Send(data.data(), static_cast<int32>(data.size()), BytesSent))
        {
            return std::unexpected(HC_ERROR(ErrorCode::SocketSendFailed, "Send failed"));
        }

        if (BytesSent != static_cast<int32>(data.size()))
        {
            LOG_WARN("Session {}: partial send ({}/{})", id_, BytesSent, data.size());
        }

        return {};
    }

    Result<void, Error> Session::send_json(std::string_view json)
    {
        Vec<u8> data(json.begin(), json.end());
        return send(std::move(data));
    }

    std::pair<String, u16> Session::addr() const
    {
        if (!Conn_)
        {
            return {"", 0};
        }

        ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        if (!Subsystem)
        {
            return {"", 0};
        }

        TSharedRef<FInternetAddr> PeerAddr = Subsystem->CreateInternetAddr();
        if (!Conn_->GetPeerAddress(*PeerAddr))
        {
            return {"", 0};
        }

        uint32 RawIp = 0;
        PeerAddr->GetIp(RawIp);

        return {Network::ToString(FIPv4Address(RawIp).ToString()),
                static_cast<u16>(PeerAddr->GetPort())};
    }

    void Session::deliver_bytes(Vec<u8> data)
    {
        usize offset = 0;
        while (offset + 4 <= data.size())
        {
            const u16 type = static_cast<u16>(data[offset] | (data[offset + 1] << 8));
            const u16 len  = static_cast<u16>(data[offset + 2] | (data[offset + 3] << 8));
            offset += 4;

            if (offset + len > data.size())
            {
                LOG_WARN("Session {}: malformed packet (len {})", id_, len);
                break;
            }

            PacketView packet;
            packet.type = type;
            packet.body.assign(data.begin() + static_cast<std::ptrdiff_t>(offset), data.begin() + static_cast<std::ptrdiff_t>(offset + len));
            offset += len;

            if (on_data_cb_)
            {
                on_data_cb_(shared_from_this(), packet);
            }
        }
    }

    void Session::notify_disconnected()
    {
        if (!active_.exchange(false))
        {
            return;
        }

        LOG_INFO("Session {} disconnected", id_);

        if (on_disconnect_cb_)
        {
            on_disconnect_cb_(shared_from_this());
        }

        server_.on_session_disconnect(id_);
    }

    void Session::notify_error(Error err)
    {
        LOG_WARN("Session {} error: {}", id_, err.message);

        if (on_error_cb_)
        {
            on_error_cb_(shared_from_this(), err);
        }
    }

    void Session::handle_packet(const PacketView& packet)
    {
        PacketHandler::handle(server_, shared_from_this(), packet);
    }
}
