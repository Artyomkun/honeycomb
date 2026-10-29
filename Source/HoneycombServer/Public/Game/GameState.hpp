#pragma once
#include "Core.hpp"
#include "Session.hpp"

namespace Honeycomb::Game
{
    using namespace Honeycomb::Core;
    using KarmaValue = i32;
    using EnergyValue = f32;
    using ChakraLevel = i32;
    using DimensionLevel = i32;
    using WorldIndex = i32;

    struct PlayerState
    {
        PlayerId id = 0;
        String nickname;

        ChakraLevel chakra_level = 1;
        DimensionLevel dimension = 3;
        WorldIndex world_index = 0;

        KarmaValue karma = 0;
        i32 reincarnation_count = 0;

        EnergyValue energy = 100.0f;
        EnergyValue max_energy = 100.0f;
        f32 health = 100.0f;
        f32 max_health = 100.0f;

        f32 pos_x = 0.0f, pos_y = 0.0f, pos_z = 0.0f;
        f32 rot_yaw = 0.0f;

        bool is_dead = false;
        i64 last_save_timestamp = 0;
        i64 created_timestamp = 0;

        f32 meditation_progress = 0.0f;
        i32 completed_meditations = 0;

        Vec<ChakraLevel> unlocked_chakras;
        Vec<WorldIndex> completed_worlds;

        void reset();
    };

    class GameState
    {
        public:
            [[nodiscard]] static PlayerState create_default(PlayerId id, std::string_view nickname);
            [[nodiscard]] static i32 required_chakra_for_dimension(DimensionLevel dimension);
            [[nodiscard]] static constexpr i32 total_chakras() noexcept { return 12; }
            [[nodiscard]] static constexpr i32 total_worlds() noexcept { return 54; }
            [[nodiscard]] static constexpr DimensionLevel max_dimension() noexcept { return 5; }
            [[nodiscard]] static f32 energy_multiplier(ChakraLevel level);
            [[nodiscard]] static String karma_tier_name(KarmaValue karma);
            [[nodiscard]] static bool can_advance_dimension(const PlayerState& state) noexcept;

            static void apply_karma(PlayerState& state, i32 delta);
            static void apply_reincarnation(PlayerState& state);
            static void regenerate_energy(PlayerState& state, f32 delta_time);
    };
}