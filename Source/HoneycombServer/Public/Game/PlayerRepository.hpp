#pragma once
#include "Core.hpp"
#include "GameState.hpp"
#include "OracleConnection.hpp"

namespace Honeycomb::Game
{
    using namespace Honeycomb::Core;

    class PlayerRepository
    {
        public:
            explicit PlayerRepository(Database::OracleConnectionPool& pool) noexcept: pool_(pool)
            {
            }

            [[nodiscard]] Result<void, Error> save(const PlayerState& state);
            [[nodiscard]] Result<Opt<PlayerState>, Error> load(PlayerId id);

        private:
            Database::OracleConnectionPool& pool_;
    };
}
