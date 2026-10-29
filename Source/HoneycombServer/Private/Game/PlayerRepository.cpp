#include "PlayerRepository.hpp"
#include "Logger.hpp"
#include <charconv>

namespace Honeycomb::Game
{
    namespace
    {
        [[nodiscard]] String JoinInts(const Vec<i32>& Values)
        {
            String Csv;
            for (usize i = 0; i < Values.size(); ++i)
            {
                if (i > 0)
                {
                    Csv.push_back(',');
                }
                Csv += std::to_string(Values[i]);
            }
            return Csv;
        }

        [[nodiscard]] Vec<i32> ParseInts(std::string_view Csv)
        {
            Vec<i32> Values;
            if (Csv.empty())
            {
                return Values;
            }

            for (const String& Token : Split(Csv, ','))
            {
                const String Item = Trim(Token);
                if (Item.empty())
                {
                    continue;
                }

                i32 Value = 0;
                if (std::from_chars(Item.data(), Item.data() + Item.size(), Value).ec == std::errc{})
                {
                    Values.push_back(Value);
                }
            }
            return Values;
        }

        [[nodiscard]] i64 ToI64(const Map<String, String>& Row, std::string_view Column, i64 Fallback = 0)
        {
            const auto it = Row.find(String(Column));
            if (it == Row.end())
            {
                return Fallback;
            }

            i64 Value = Fallback;
            const String& Text = it->second;
            if (std::from_chars(Text.data(), Text.data() + Text.size(), Value).ec != std::errc{})
            {
                return Fallback;
            }
            return Value;
        }

        [[nodiscard]] double ToDouble(const Map<String, String>& Row, std::string_view Column, double Fallback = 0.0)
        {
            const auto it = Row.find(String(Column));
            if (it == Row.end())
            {
                return Fallback;
            }

            double Value = Fallback;
            const String& Text = it->second;
            if (std::from_chars(Text.data(), Text.data() + Text.size(), Value).ec != std::errc{})
            {
                return Fallback;
            }
            return Value;
        }

        [[nodiscard]] String ToText(const Map<String, String>& Row, std::string_view Column)
        {
            const auto it = Row.find(String(Column));
            return it == Row.end() ? String{} : it->second;
        }

        constexpr const char* MergePlayerSql = R"SQL(
MERGE INTO HC_PLAYERS p
USING (SELECT :1 AS PLAYER_ID FROM DUAL) src
ON (p.PLAYER_ID = src.PLAYER_ID)
WHEN MATCHED THEN UPDATE SET
    p.NICKNAME = :2,
    p.CHAKRA_LEVEL = :3,
    p.DIMENSION = :4,
    p.WORLD_INDEX = :5,
    p.KARMA = :6,
    p.REINCARNATION_COUNT = :7,
    p.ENERGY = :8,
    p.MAX_ENERGY = :9,
    p.HEALTH = :10,
    p.MAX_HEALTH = :11,
    p.POS_X = :12,
    p.POS_Y = :13,
    p.POS_Z = :14,
    p.ROT_YAW = :15,
    p.IS_DEAD = :16,
    p.MEDITATION_PROGRESS = :17,
    p.COMPLETED_MEDITATIONS = :18,
    p.UNLOCKED_CHAKRAS = :19,
    p.COMPLETED_WORLDS = :20,
    p.CREATED_TS = :21,
    p.LAST_SAVE_TS = :22
WHEN NOT MATCHED THEN INSERT (
    PLAYER_ID, NICKNAME, CHAKRA_LEVEL, DIMENSION, WORLD_INDEX, KARMA,
    REINCARNATION_COUNT, ENERGY, MAX_ENERGY, HEALTH, MAX_HEALTH,
    POS_X, POS_Y, POS_Z, ROT_YAW, IS_DEAD, MEDITATION_PROGRESS,
    COMPLETED_MEDITATIONS, UNLOCKED_CHAKRAS, COMPLETED_WORLDS,
    CREATED_TS, LAST_SAVE_TS)
VALUES (
    :1, :2, :3, :4, :5, :6, :7, :8, :9, :10, :11,
    :12, :13, :14, :15, :16, :17, :18, :19, :20, :21, :22)
)SQL";

        constexpr const char* SelectPlayerSql = R"SQL(
SELECT PLAYER_ID, NICKNAME, CHAKRA_LEVEL, DIMENSION, WORLD_INDEX, KARMA,
    REINCARNATION_COUNT, ENERGY, MAX_ENERGY, HEALTH, MAX_HEALTH,
    POS_X, POS_Y, POS_Z, ROT_YAW, IS_DEAD, MEDITATION_PROGRESS,
    COMPLETED_MEDITATIONS, UNLOCKED_CHAKRAS, COMPLETED_WORLDS,
    CREATED_TS, LAST_SAVE_TS
FROM HC_PLAYERS
WHERE PLAYER_ID = :1
)SQL";
    }

    Result<void, Error> PlayerRepository::save(const PlayerState& state)
    {
        auto conn = pool_.acquire();
        if (!conn)
        {
            return std::unexpected(conn.error());
        }

        auto Saved = (*conn)->execute(MergePlayerSql, {
            static_cast<i64>(state.id),
            state.nickname,
            static_cast<i64>(state.chakra_level),
            static_cast<i64>(state.dimension),
            static_cast<i64>(state.world_index),
            static_cast<i64>(state.karma),
            static_cast<i64>(state.reincarnation_count),
            static_cast<double>(state.energy),
            static_cast<double>(state.max_energy),
            static_cast<double>(state.health),
            static_cast<double>(state.max_health),
            static_cast<double>(state.pos_x),
            static_cast<double>(state.pos_y),
            static_cast<double>(state.pos_z),
            static_cast<double>(state.rot_yaw),
            static_cast<i64>(state.is_dead ? 1 : 0),
            static_cast<double>(state.meditation_progress),
            static_cast<i64>(state.completed_meditations),
            JoinInts(state.unlocked_chakras),
            JoinInts(state.completed_worlds),
            static_cast<i64>(state.created_timestamp),
            static_cast<i64>(state.last_save_timestamp),
        });

        pool_.release(*conn);

        if (!Saved)
        {
            return std::unexpected(Saved.error());
        }
        return {};
    }

    Result<Opt<PlayerState>, Error> PlayerRepository::load(PlayerId id)
    {
        auto conn = pool_.acquire();
        if (!conn)
        {
            return std::unexpected(conn.error());
        }

        auto Rows = (*conn)->query(SelectPlayerSql, { static_cast<i64>(id) });
        pool_.release(*conn);

        if (!Rows)
        {
            return std::unexpected(Rows.error());
        }

        if (Rows->empty())
        {
            return Opt<PlayerState>{std::nullopt};
        }

        const Map<String, String>& Row = Rows->front();

        PlayerState state;
        state.id = static_cast<PlayerId>(ToI64(Row, "PLAYER_ID", static_cast<i64>(id)));
        state.nickname = ToText(Row, "NICKNAME");
        state.chakra_level = static_cast<ChakraLevel>(ToI64(Row, "CHAKRA_LEVEL", 1));
        state.dimension = static_cast<DimensionLevel>(ToI64(Row, "DIMENSION", 3));
        state.world_index = static_cast<WorldIndex>(ToI64(Row, "WORLD_INDEX", 0));
        state.karma = static_cast<KarmaValue>(ToI64(Row, "KARMA", 0));
        state.reincarnation_count = static_cast<i32>(ToI64(Row, "REINCARNATION_COUNT", 0));
        state.energy = static_cast<f32>(ToDouble(Row, "ENERGY", 100.0));
        state.max_energy = static_cast<f32>(ToDouble(Row, "MAX_ENERGY", 100.0));
        state.health = static_cast<f32>(ToDouble(Row, "HEALTH", 100.0));
        state.max_health = static_cast<f32>(ToDouble(Row, "MAX_HEALTH", 100.0));
        state.pos_x = static_cast<f32>(ToDouble(Row, "POS_X", 0.0));
        state.pos_y = static_cast<f32>(ToDouble(Row, "POS_Y", 0.0));
        state.pos_z = static_cast<f32>(ToDouble(Row, "POS_Z", 0.0));
        state.rot_yaw = static_cast<f32>(ToDouble(Row, "ROT_YAW", 0.0));
        state.is_dead = ToI64(Row, "IS_DEAD", 0) != 0;
        state.meditation_progress = static_cast<f32>(ToDouble(Row, "MEDITATION_PROGRESS", 0.0));
        state.completed_meditations = static_cast<i32>(ToI64(Row, "COMPLETED_MEDITATIONS", 0));
        state.unlocked_chakras = ParseInts(ToText(Row, "UNLOCKED_CHAKRAS"));
        state.completed_worlds = ParseInts(ToText(Row, "COMPLETED_WORLDS"));
        state.created_timestamp = ToI64(Row, "CREATED_TS", 0);
        state.last_save_timestamp = ToI64(Row, "LAST_SAVE_TS", 0);

        return Opt<PlayerState>{std::move(state)};
    }
}

