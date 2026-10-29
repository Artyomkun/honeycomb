#pragma once
#include "GameState.hpp"

namespace Honeycomb::Game
{
    using namespace Honeycomb::Core;
    class GameServer;

    class GameWorld
    {
        public:
            explicit GameWorld(GameServer& server);

            Result<PlayerState*, Error> get_player(PlayerId id);
            Result<const PlayerState*, Error> get_player(PlayerId id) const;
            Result<PlayerState*, Error> get_player_by_session(SessionId session_id);

            [[nodiscard]] Result<PlayerState, Error> get_player_copy(PlayerId id) const;

            [[nodiscard]] Result<PlayerState, Error> mutate_player(PlayerId id, const std::function<void(PlayerState&)>& mutator);

            void add_player(PlayerState state);
            void remove_player(PlayerId id);
            void update_player(PlayerState state);

            [[nodiscard]] const Map<PlayerId, PlayerState>& players() const noexcept { return players_; }
            [[nodiscard]] usize player_count() const noexcept { return players_.size(); }

            void tick(f32 delta_time);

        private:
            GameServer& server_;
            Map<PlayerId, PlayerState> players_;
            Map<SessionId, PlayerId> session_to_player_;
            mutable std::shared_mutex mutex_;

            f32 accumulator_ = 0.0f;
            static constexpr f32 tick_rate_ = 30.0f;
    };
}