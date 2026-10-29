#include "GameWorld.hpp"
#include "Server.hpp"
#include "Logger.hpp"

namespace Honeycomb::Game
{
    GameWorld::GameWorld(GameServer& server)
        : server_(server)
    {
    }

    Result<PlayerState*, Error> GameWorld::get_player(PlayerId id)
    {
        std::shared_lock lock(mutex_);
        auto it = players_.find(id);
        if (it == players_.end())
        {
            return std::unexpected(HC_ERROR(ErrorCode::GamePlayerNotFound, "Player not found"));
        }
        return &it->second;
    }

    Result<const PlayerState*, Error> GameWorld::get_player(PlayerId id) const
    {
        std::shared_lock lock(mutex_);
        auto it = players_.find(id);
        if (it == players_.end())
        {
            return std::unexpected(HC_ERROR(ErrorCode::GamePlayerNotFound, "Player not found"));
        }
        return &it->second;
    }

    Result<PlayerState*, Error> GameWorld::get_player_by_session(SessionId session_id)
    {
        std::shared_lock lock(mutex_);
        auto sit = session_to_player_.find(session_id);
        if (sit == session_to_player_.end())
        {
            return std::unexpected(HC_ERROR(ErrorCode::GamePlayerNotFound, "Session not mapped"));
        }
        auto pit = players_.find(sit->second);
        if (pit == players_.end())
        {
            return std::unexpected(HC_ERROR(ErrorCode::GamePlayerNotFound, "Player not found"));
        }
        return &pit->second;
    }

    void GameWorld::add_player(PlayerState state)
    {
        std::unique_lock lock(mutex_);
        players_[state.id] = std::move(state);
    }

    void GameWorld::remove_player(PlayerId id)
    {
        std::unique_lock lock(mutex_);
        players_.erase(id);
        std::erase_if(session_to_player_,
            [id](const auto& kv) { return kv.second == id; });
    }

    void GameWorld::update_player(PlayerState state)
    {
        std::unique_lock lock(mutex_);
        if (auto it = players_.find(state.id); it != players_.end())
        {
            it->second = std::move(state);
        }
    }

    Result<PlayerState, Error> GameWorld::get_player_copy(PlayerId id) const
    {
        std::shared_lock lock(mutex_);
        auto it = players_.find(id);
        if (it == players_.end())
        {
            return std::unexpected(HC_ERROR(ErrorCode::GamePlayerNotFound, "Player not found"));
        }
        return it->second;
    }

    Result<PlayerState, Error> GameWorld::mutate_player(PlayerId id, const std::function<void(PlayerState&)>& mutator)
    {
        std::unique_lock lock(mutex_);
        auto it = players_.find(id);
        if (it == players_.end())
        {
            return std::unexpected(HC_ERROR(ErrorCode::GamePlayerNotFound, "Player not found"));
        }

        if (mutator)
        {
            mutator(it->second);
        }

        return it->second;
    }

    void GameWorld::tick(f32 delta_time)
    {
        accumulator_ += delta_time;
        if (accumulator_ < 1.0f / tick_rate_) return;
        accumulator_ = 0.0f;

        std::unique_lock lock(mutex_);
        for (auto& [id, state]: players_)
        {
            GameState::regenerate_energy(state, 1.0f / tick_rate_);
        }
    }
}