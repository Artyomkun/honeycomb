#include "GameState.hpp"

namespace Honeycomb::Game
{
    void PlayerState::reset()
    {
        chakra_level = 1;
        dimension = 3;
        world_index = 0;
        karma = 0;
        reincarnation_count = 0;
        energy = 100.0f;
        max_energy = 100.0f;
        health = 100.0f;
        max_health = 100.0f;
        pos_x = pos_y = pos_z = 0.0f;
        rot_yaw = 0.0f;
        is_dead = false;
        meditation_progress = 0.0f;
        completed_meditations = 0;
        unlocked_chakras.clear();
        completed_worlds.clear();
    }

    PlayerState GameState::create_default(PlayerId id, std::string_view nickname)
    {
        PlayerState state;
        state.id = id;
        state.nickname = nickname;
        state.created_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        state.last_save_timestamp = state.created_timestamp;
        state.unlocked_chakras.push_back(1);
        return state;
    }

    i32 GameState::required_chakra_for_dimension(DimensionLevel dimension)
    {
        switch (dimension)
        {
            case 1:
                return 1;
            case 2:
                return 3;
            case 3:
                return 5;
            case 4:
                return 8;
            case 5:
                return 12;
            default:
                return 12;
        }
    }

    f32 GameState::energy_multiplier(ChakraLevel level)
    {
        return 1.0f + (level - 1) * 0.1f;
    }

    String GameState::karma_tier_name(KarmaValue karma)
    {
        if (karma <= -500) {
            return "Dark";
        }
        if (karma <= -100) {
            return "Shadow";
        }
        if (karma < 100)  {
            return "Neutral";
        }
        if (karma < 500) {
            return "Light";
        }
        return "Radiant";
    }

    bool GameState::can_advance_dimension(const PlayerState& state) noexcept
    {
        return state.chakra_level >= required_chakra_for_dimension(state.dimension + 1);
    }

    void GameState::apply_karma(PlayerState& state, i32 delta)
    {
        state.karma += delta;
        state.karma = std::max<KarmaValue>(-1000, std::min<KarmaValue>(1000, state.karma));
    }

    void GameState::apply_reincarnation(PlayerState& state)
    {
        ++state.reincarnation_count;
        i32 rebirth_count = state.reincarnation_count;
        state.reset();
        state.reincarnation_count = rebirth_count;
    }

    void GameState::regenerate_energy(PlayerState& state, f32 delta_time)
    {
        f32 regen_rate = 5.0f * energy_multiplier(state.chakra_level);
        if (state.meditation_progress > 0.0f)
        {
            regen_rate *= 2.0f;
        }
        state.energy = std::min(state.energy + regen_rate * delta_time, state.max_energy);
    }
}