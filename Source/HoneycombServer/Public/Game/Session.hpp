#pragma once
#include "Core.hpp"
#include "FSocketRef.hpp"
#include <functional>

namespace Honeycomb::Game
{
    using namespace Honeycomb::Core;
    using SessionId = u64;
    using PlayerId = u64;

    struct PacketView
    {
        u16 type = 0;
        Vec<u8> body;
    };

    class GameServer;

    class Session : public std::enable_shared_from_this<Session>
    {
        public:
            using Ptr = std::shared_ptr<Session>;

            Session(SessionId id, Network::FSocketRef Conn, GameServer& server);
            ~Session() = default;

            Session(const Session&) = delete;
            Session& operator=(const Session&) = delete;

            void start();
            void shut_down();

            Result<void, Error> send(Vec<u8> data);
            Result<void, Error> send_json(std::string_view json);

            void deliver_bytes(Vec<u8> data);
            void notify_disconnected();
            void notify_error(Error err);

            [[nodiscard]] bool is_active() const noexcept { return active_; }
            [[nodiscard]] FSocket* get_native_socket() const noexcept { return Conn_ ? Conn_.Get() : nullptr; }
            [[nodiscard]] Network::FSocketRef get_conn() const noexcept { return Conn_; }

            [[nodiscard]] SessionId id() const noexcept { return id_; }
            [[nodiscard]] Opt<PlayerId> player_id() const noexcept { return player_id_; }
            [[nodiscard]] std::pair<String, u16> addr() const;
            [[nodiscard]] bool is_authenticated() const noexcept { return player_id_.has_value(); }

            void set_player_id(PlayerId pid) noexcept { player_id_ = pid; }
            void set_id(SessionId id) noexcept { id_ = id; }

            void set_on_disconnect(std::function<void(Ptr)> cb) { on_disconnect_cb_ = std::move(cb); }
            void set_on_data(std::function<void(Ptr, const PacketView&)> cb) { on_data_cb_ = std::move(cb); }
            void set_on_error(std::function<void(Ptr, Error)> cb) { on_error_cb_ = std::move(cb); }

        private:
            SessionId id_ = 0;
            Opt<PlayerId> player_id_ = std::nullopt;
            Network::FSocketRef Conn_;
            GameServer& server_;
            std::atomic<bool> active_{true};

            std::function<void(Ptr)> on_disconnect_cb_;
            std::function<void(Ptr, const PacketView&)> on_data_cb_;
            std::function<void(Ptr, Error)> on_error_cb_;

            void handle_packet(const PacketView& packet);
    };
}
