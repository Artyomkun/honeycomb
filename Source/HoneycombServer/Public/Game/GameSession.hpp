#pragma once
#include "Session.hpp"

namespace Honeycomb::Game
{
    using namespace Honeycomb::Core;
    class GameServer;
    struct PlayerState;

    class GameSession
    {
        public:
            GameSession(SessionId id, Network::FSocketRef socket, GameServer& server);
            GameSession(const GameSession&) = delete;
            GameSession& operator=(const GameSession&) = delete;

            void on_player_joined(Session::Ptr session);
            void on_player_left(PlayerId pid);

            Result<void, Error> save_player_state(SessionId id);
            Result<void, Error> load_player_state(SessionId id);

            Vec<u8> make_auth_response(bool success, std::string_view message, PlayerId pid);
            Vec<u8> make_state_sync(const PlayerState& state);

        private:
            SessionId id_ = 0;
            Network::FSocketRef socket_;
            GameServer& server_;
            Opt<PlayerId> player_id_ = std::nullopt;
            bool active_ = true;
            std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();
    };

    namespace Protocol
    {
        class Writer
        {
            public:
                void write_u8(u8 v);
                void write_u16(u16 v);
                void write_u32(u32 v);
                void write_u64(u64 v);
                void write_f32(f32 v);
                void write_f64(f64 v);
                void string(std::string_view s);

                [[nodiscard]] Vec<u8> bytes() const& { return buffer_; }
                [[nodiscard]] Vec<u8> bytes() && { return std::move(buffer_); }
            private:
                Vec<u8> buffer_;
        };

        class Reader
        {
            public:
                explicit Reader(std::span<const u8> data) : data_(data) {}

                Result<u8, Error> read_u8();
                Result<u16, Error> read_u16();
                Result<u32, Error> read_u32();
                Result<u64, Error> read_u64();
                Result<f32, Error> read_f32();
                Result<f64, Error> read_f64();
                Result<StringView, Error> string_view();

                [[nodiscard]] bool eof() const noexcept { return pos_ >= data_.size(); }
                [[nodiscard]] usize remaining() const noexcept { return data_.size() - pos_; }
            private:
                std::span<const u8> data_;
                usize pos_ = 0;
        };
    }
}