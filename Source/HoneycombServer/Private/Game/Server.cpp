#include "Server.hpp"
#include "GameSession.hpp"
#include "Socket.hpp"
#include "Logger.hpp"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Common/TcpListener.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"

namespace Honeycomb::Game
{
    GameServer::GameServer(ServerConfig config): config_(std::move(config)), db_pool_(config_.database), world_(*this) {}

    GameServer::~GameServer()
    {
        LogAndContinue("server stop", stop());
        db_pool_.close_all();
    }

    Result<void, Error> GameServer::start()
    {
        if (running_.exchange(true))
        {
            return std::unexpected(HC_ERROR(ErrorCode::GameInvalidState, "Already running"));
        }

        FIPv4Address IpAddr;
        if (!FIPv4Address::Parse(FString(config_.bind_address.c_str()), IpAddr))
        {
            FIPv4Address::Parse(TEXT("0.0.0.0"), IpAddr);
        }

        const FIPv4Endpoint Endpoint(IpAddr, config_.tcp_port);
        tcp_listener_ = MakeUnique<FTcpListener>(Endpoint, FTimespan::FromMilliseconds(5), true);

        if (!tcp_listener_->IsActive())
        {
            LOG_ERROR("Failed to start TCP listener on {}:{}", config_.bind_address, config_.tcp_port);
            tcp_listener_.Reset();
            running_ = false;
            return std::unexpected(HC_ERROR(ErrorCode::SocketListenFailed, "Failed to start TCP listener"));
        }

        tcp_listener_->OnConnectionAccepted().BindLambda(
            [this](FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint) -> bool
            {
                return on_connection_accepted(ClientSocket);
            });

        LOG_INFO("Game TCP server listening on {}:{}", config_.bind_address, config_.tcp_port);

        ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        if (!Subsystem)
        {
            LOG_WARN("Socket subsystem not available, UDP disabled");
        }
        else
        {
            udp_socket_ = Network::MakeSocketRef(
                Subsystem->CreateSocket(NAME_DGram, TEXT("HoneycombUDP"), false), Subsystem);

            TSharedRef<FInternetAddr> UdpAddr = Subsystem->CreateInternetAddr();
            UdpAddr->SetIp(IpAddr.Value);
            UdpAddr->SetPort(static_cast<int32>(config_.udp_port));

            if (!udp_socket_ || !udp_socket_->Bind(*UdpAddr))
            {
                LOG_WARN("UDP socket failed to start: Bind failed");
                udp_socket_.Reset();
            }
            else
            {
                LOG_INFO("Game UDP socket on {}:{}", config_.bind_address, config_.udp_port);
            }
        }

        if (FHttpServerModule::IsAvailable())
        {
            FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
            http_router_ = HttpServerModule.GetHttpRouter(config_.http_port);

            if (http_router_)
            {
                init_http_routes();
                HttpServerModule.StartAllListeners();
                LOG_INFO("HTTP server started on port {}", config_.http_port);
            }
            else
            {
                LOG_WARN("HTTP router not available on port {}", config_.http_port);
            }
        }
        else
        {
            LOG_WARN("FHttpServerModule not available");
        }

        workers_.emplace_back([this] { io_loop(); });

        if (udp_socket_)
        {
            workers_.emplace_back([this] { udp_loop(); });
        }

        LOG_INFO("Honeycomb game server started");
        return {};
    }

    Result<void, Error> GameServer::stop()
    {
        if (!running_.exchange(false))
        {
            return {};
        }

        const auto SelfId = std::this_thread::get_id();
        for (Thread& Worker : workers_)
        {
            if (Worker.joinable() && Worker.get_id() != SelfId)
            {
                Worker.join();
            }
        }
        workers_.clear();

        tcp_listener_.Reset();

        if (http_router_ && FHttpServerModule::IsAvailable())
        {
            FHttpServerModule::Get().StopAllListeners();
            http_router_.Reset();
        }

        if (udp_socket_)
        {
            udp_socket_->Close();
            udp_socket_.Reset();
        }

        Vec<Session::Ptr> Sessions;
        {
            std::lock_guard lock(sessions_mutex_);
            Sessions.reserve(sessions_.size());
            for (auto& [_, session] : sessions_)
            {
                Sessions.push_back(session);
            }
            sessions_.clear();
            player_to_session_.clear();
        }

        for (Session::Ptr& session: Sessions)
        {
            if (session)
            {
                session->shut_down();
            }
        }

        return {};
    }

    bool GameServer::on_connection_accepted(FSocket* ClientSocket)
    {
        if (!ClientSocket)
        {
            return false;
        }

        const SessionId Sid = next_session_id();

        {
            std::lock_guard lock(sessions_mutex_);
            if (static_cast<i32>(sessions_.size()) >= config_.max_players)
            {
                LOG_WARN("Server full ({} players), rejecting session {}", sessions_.size(), Sid);
                return false;
            }
        }

        Network::FSocketRef Conn = Network::MakeSocketRef(ClientSocket);
        if (!Conn)
        {
            return false;
        }

        Session::Ptr session = std::make_shared<Session>(Sid, Conn, *this);

        session->set_on_disconnect([this](Session::Ptr S) { on_session_disconnect(S->id()); });
        session->set_on_data([this](Session::Ptr S, const PacketView& Packet) { on_session_data(S, Packet); });

        {
            std::lock_guard lock(sessions_mutex_);
            sessions_[Sid] = session;
        }

        session->start();
        return true;
    }

    void GameServer::io_loop()
    {
        Vec<Session::Ptr> Snapshot;

        while (running_)
        {
            {
                std::lock_guard lock(sessions_mutex_);
                Snapshot.clear();
                Snapshot.reserve(sessions_.size());
                for (auto& [_, session] : sessions_)
                {
                    Snapshot.push_back(session);
                }
            }

            for (const Session::Ptr& session : Snapshot)
            {
                if (!session || !session->is_active())
                {
                    continue;
                }

                FSocket* NativeSocket = session->get_native_socket();
                if (!NativeSocket)
                {
                    continue;
                }

                uint32 PendingSize = 0;
                if (!NativeSocket->HasPendingData(PendingSize) || PendingSize == 0)
                {
                    continue;
                }

                Vec<u8> Buffer(static_cast<usize>(PendingSize));
                int32 BytesRead = 0;

                if (!NativeSocket->Recv(Buffer.data(), static_cast<int32>(Buffer.size()), BytesRead))
                {
                    session->notify_disconnected();
                    continue;
                }

                if (BytesRead <= 0)
                {
                    session->notify_disconnected();
                    continue;
                }

                Buffer.resize(static_cast<usize>(BytesRead));
                session->deliver_bytes(std::move(Buffer));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    void GameServer::on_session_data(Session::Ptr session, const PacketView& packet)
    {
        PacketHandler::handle(*this, session, packet);
    }

    void GameServer::on_session_disconnect(SessionId id)
    {
        Session::Ptr Removed;
        {
            std::lock_guard lock(sessions_mutex_);
            auto it = sessions_.find(id);
            if (it == sessions_.end())
            {
                return;
            }

            Removed = it->second;
            sessions_.erase(it);

            if (Removed && Removed->player_id())
            {
                player_to_session_.erase(*Removed->player_id());
            }
        }

        if (Removed && Removed->player_id())
        {
            world_.remove_player(*Removed->player_id());
        }

        LOG_INFO("Session {} disconnected", id);
    }

    Opt<Session::Ptr> GameServer::find_session(SessionId id)
    {
        std::lock_guard lock(sessions_mutex_);
        auto it = sessions_.find(id);
        if (it == sessions_.end()) return std::nullopt;
        return it->second;
    }

    void GameServer::broadcast(Vec<u8> packet)
    {
        Vec<Session::Ptr> Targets;
        {
            std::lock_guard lock(sessions_mutex_);
            Targets.reserve(sessions_.size());
            for (auto& [_, session] : sessions_)
            {
                Targets.push_back(session);
            }
        }

        for (const Session::Ptr& session : Targets)
        {
            if (session)
            {
                LogAndContinue("broadcast", session->send(packet));
            }
        }
    }

    void GameServer::broadcast_except(SessionId except, Vec<u8> packet)
    {
        Vec<Session::Ptr> Targets;
        {
            std::lock_guard lock(sessions_mutex_);
            Targets.reserve(sessions_.size());
            for (auto& [id, session] : sessions_)
            {
                if (id != except)
                {
                    Targets.push_back(session);
                }
            }
        }

        for (const Session::Ptr& session : Targets)
        {
            if (session)
            {
                LogAndContinue("broadcast-except", session->send(packet));
            }
        }
    }

    void GameServer::send_to_session(SessionId id, Vec<u8> packet)
    {
        if (auto session = find_session(id))
        {
            LogAndContinue("send to session", (*session)->send(std::move(packet)));
        }
    }

    void GameServer::udp_loop()
    {
        ISocketSubsystem* Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        if (!udp_socket_ || !Subsystem)
        {
            LOG_WARN("UDP socket not available");
            return;
        }

        TSharedRef<FInternetAddr> SourceAddr = Subsystem->CreateInternetAddr();

        while (running_)
        {
            FSocket* Socket = udp_socket_.Get();
            if (!Socket)
            {
                break;
            }

            uint32 PendingSize = 0;
            if (!Socket->HasPendingData(PendingSize) || PendingSize == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            Vec<u8> Buffer(static_cast<usize>(PendingSize));
            int32 BytesRead = 0;

            if (!Socket->RecvFrom(Buffer.data(), static_cast<int32>(Buffer.size()), BytesRead, *SourceAddr))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            Buffer.resize(static_cast<usize>(BytesRead));

            uint32 RawIp = 0;
            SourceAddr->GetIp(RawIp);

            const String Addr = Network::ToString(FIPv4Address(RawIp).ToString());
            const u16 Port = static_cast<u16>(SourceAddr->GetPort());

            LOG_DEBUG("UDP datagram {} bytes from {}:{}", BytesRead, Addr, Port);
        }
    }

    void GameServer::init_http_routes()
    {
        if (!http_router_) return;

        http_router_->BindRoute(
            FHttpPath(TEXT("/health")),
            EHttpServerRequestVerbs::VERB_GET,
            FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest&, const FHttpResultCallback& OnComplete) -> bool
            {
                TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(
                    TEXT("{\"status\":\"ok\"}"),
                    TEXT("application/json")
                );
                OnComplete(MoveTemp(Response));
                return true;
            })
        );

        http_router_->BindRoute(
            FHttpPath(TEXT("/api/auth/login")),
            EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
            {
                return handle_auth_http(Request, OnComplete);
            })
        );

        http_router_->BindRoute(
            FHttpPath(TEXT("/api/save")),
            EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
            {
                auto conn = db_pool_.acquire();
                if (!conn)
                {
                    TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Error(
                        EHttpServerResponseCodes::ServerError,
                        TEXT("database_error"),
                        TEXT("Database unavailable")
                    );
                    OnComplete(MoveTemp(Response));
                    return true;
                }

                const bool bHandled = handle_save_http(**conn, Request, OnComplete);
                db_pool_.release(*conn);
                return bHandled;
            })
        );

        http_router_->BindRoute(
            FHttpPath(TEXT("/api/load")),
            EHttpServerRequestVerbs::VERB_POST,
            FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
            {
                auto conn = db_pool_.acquire();
                if (!conn)
                {
                    TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Error(
                        EHttpServerResponseCodes::ServerError,
                        TEXT("database_error"),
                        TEXT("Database unavailable")
                    );
                    OnComplete(MoveTemp(Response));
                    return true;
                }

                const bool bHandled = handle_load_http(**conn, Request, OnComplete);
                db_pool_.release(*conn);
                return bHandled;
            })
        );
    }

    bool GameServer::handle_auth_http(const FHttpServerRequest&, const FHttpResultCallback& OnComplete)
    {
        TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(
            TEXT("{\"token\":\"dev-token-123\",\"message\":\"login ok\"}"),
            TEXT("application/json")
        );
        OnComplete(MoveTemp(Response));
        return true;
    }

    bool GameServer::handle_save_http(Database::OracleConnection& conn, const FHttpServerRequest&, const FHttpResultCallback& OnComplete)
    {
        auto ping = conn.ping();
        if (!ping)
        {
            TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Error(
                EHttpServerResponseCodes::ServerError,
                TEXT("db_error"),
                TEXT("DB not ready")
            );
            OnComplete(MoveTemp(Response));
            return true;
        }

        TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(
            TEXT("{\"saved\":true}"),
            TEXT("application/json")
        );
        OnComplete(MoveTemp(Response));
        return true;
    }

    bool GameServer::handle_load_http(Database::OracleConnection& conn, const FHttpServerRequest&, const FHttpResultCallback& OnComplete)
    {
        auto ping = conn.ping();
        if (!ping)
        {
            TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Error(
                EHttpServerResponseCodes::ServerError,
                TEXT("db_error"),
                TEXT("DB not ready")
            );
            OnComplete(MoveTemp(Response));
            return true;
        }

        TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(
            TEXT("{\"loaded\":true,\"state\":{}}"),
            TEXT("application/json")
        );
        OnComplete(MoveTemp(Response));
        return true;
    }
}

