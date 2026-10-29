#pragma once
#include "Session.hpp"
#include "OracleConnection.hpp"
#include "GameWorld.hpp"
#include <functional>
#include "Sockets.h"
#include "Common/TcpListener.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "HttpResultCallback.h"
#include "HttpRequestHandler.h"
#include "HttpRouteHandle.h"

namespace Honeycomb::Game
{
    using namespace Honeycomb::Core;
    enum class PacketType: u16
    {
        Heartbeat         = 0x0001,
        AuthLogin         = 0x0101,
        AuthRegister       = 0x0102,
        AuthLogout        = 0x0103,
        PlayerUpdate      = 0x0201,
        PlayerMove        = 0x0202,
        PlayerStateSync   = 0x0203,
        SaveGameRequest   = 0x0301,
        SaveGameResponse  = 0x0302,
        LoadGameRequest   = 0x0303,
        LoadGameResponse  = 0x0304,
        WorldEvent        = 0x0401,
        SystemSync        = 0x0501,
        ChatMessage       = 0x0601,
        ErrorResponse     = 0xFFFF,
    };

    struct ServerConfig
    {
        String bind_address = "0.0.0.0";
        u16 tcp_port = 7777;
        u16 udp_port = 7778;
        u16 http_port = 8080;
        u16 ws_port = 8081;
        i32 max_players = 128;
        i32 worker_threads = 4;
        Database::OracleConfig database;
    };

    class GameServer
    {
        public:
            explicit GameServer(ServerConfig config);
            ~GameServer();

            GameServer(const GameServer&) = delete;
            GameServer& operator=(const GameServer&) = delete;

            [[nodiscard]] Result<void, Error> start();
            [[nodiscard]] Result<void, Error> stop();

            void broadcast(Vec<u8> packet);
            void broadcast_except(SessionId except, Vec<u8> packet);
            void send_to_session(SessionId id, Vec<u8> packet);

            void map_player_session(PlayerId pid, SessionId sid)
            {
                std::lock_guard lock(sessions_mutex_);
                player_to_session_[pid] = sid;
            }

            [[nodiscard]] Opt<Session::Ptr> find_session(SessionId id);

            Database::OracleConnectionPool& db_pool() noexcept { return db_pool_; }
            GameWorld& world() noexcept { return world_; }
            const GameWorld& world() const noexcept { return world_; }

            [[nodiscard]] SessionId next_session_id() noexcept { return ++session_id_seq_; }
            [[nodiscard]] PlayerId next_player_id() noexcept { return ++player_id_seq_; }

            [[nodiscard]] const ServerConfig& config() const noexcept { return config_; }

        private:
            ServerConfig config_;
            Database::OracleConnectionPool db_pool_;
            GameWorld world_;

            TUniquePtr<FTcpListener> tcp_listener_;
            Network::FSocketRef udp_socket_;
            TSharedPtr<IHttpRouter> http_router_;

            std::mutex sessions_mutex_;
            Map<SessionId, Session::Ptr> sessions_;
            Map<PlayerId, SessionId> player_to_session_;

            Vec<Thread> workers_;

            std::atomic<u64> session_id_seq_{0};
            std::atomic<u64> player_id_seq_{0};
            std::atomic<bool> running_{false};

            bool on_connection_accepted(FSocket* ClientSocket);
            void io_loop();
            void udp_loop();

            void on_session_data(Session::Ptr session, const PacketView& packet);
            void on_session_disconnect(SessionId id);

            bool handle_auth_http(const FHttpServerRequest& req, const FHttpResultCallback& OnComplete);
            bool handle_save_http(Database::OracleConnection& conn, const FHttpServerRequest& req, const FHttpResultCallback& OnComplete);
            bool handle_load_http(Database::OracleConnection& conn, const FHttpServerRequest& req, const FHttpResultCallback& OnComplete);

            void init_http_routes();

            friend class Session;
    };

    class PacketHandler
    {
        public:
            static void handle(GameServer& server, Session::Ptr session, const PacketView& packet);
    };
}
