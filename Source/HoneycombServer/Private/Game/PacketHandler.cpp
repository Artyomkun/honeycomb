#include "Server.hpp"
#include "GameSession.hpp"
#include "PlayerRepository.hpp"
#include "Logger.hpp"

namespace Honeycomb::Game
{
    using Protocol::Reader;
    using Protocol::Writer;

    namespace
    {
        [[nodiscard]] bool IsOwner(Session::Ptr session, PlayerId pid)
        {
            return session->is_authenticated() && session->player_id() && *session->player_id() == pid;
        }

        [[nodiscard]] i64 NowMillis()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        }

        void send_save_response(Session::Ptr session, bool ok)
        {
            Writer w;
            w.write_u16(static_cast<u16>(PacketType::SaveGameResponse));
            w.write_u8(ok ? 1 : 0);
            LogAndContinue("send save-game response", session->send(std::move(w).bytes()));
        }

        void send_load_response(Session::Ptr session, bool ok)
        {
            Writer w;
            w.write_u16(static_cast<u16>(PacketType::LoadGameResponse));
            w.write_u8(ok ? 1 : 0);
            LogAndContinue("send load-game response", session->send(std::move(w).bytes()));
        }

        void handle_auth_login(GameServer& server, Session::Ptr session, const PacketView& packet)
        {
            Reader r(packet.body);
            auto nickname = r.string_view();
            if (!nickname)
            {
                LogAndContinue("send bad-login response", session->send(GameSession(session->id(), session->get_conn(), server).make_auth_response(false, "Bad login packet", 0)));
                return;
            }

            PlayerId pid = server.next_player_id();

            auto state = GameState::create_default(pid, *nickname);
            server.world().add_player(state);

            session->set_player_id(pid);
            server.map_player_session(pid, session->id());

            LogAndContinue("send auth response", session->send(GameSession(session->id(), session->get_conn(), server).make_auth_response(true, "Welcome", pid)));
            auto sync = GameSession(session->id(), session->get_conn(), server).make_state_sync(state);
            LogAndContinue("send auth state sync", session->send(std::move(sync)));

            LOG_INFO("Player '{}' authenticated as {}", *nickname, pid);
        }

        void handle_heartbeat(GameServer&, Session::Ptr session, const PacketView&)
        {
            Writer w;
            w.write_u16(static_cast<u16>(PacketType::Heartbeat));
            LogAndContinue("send heartbeat", session->send(std::move(w).bytes()));
        }

        void handle_player_update(GameServer& server, Session::Ptr session, const PacketView& packet)
        {
            Reader r(packet.body);
            auto pid = r.read_u64();
            if (!pid || !IsOwner(session, *pid)) return;

            const auto pos_x = r.read_f32();
            const auto pos_y = r.read_f32();
            const auto pos_z = r.read_f32();
            const auto yaw = r.read_f32();
            const auto energy = r.read_f32();
            const auto health = r.read_f32();

            if (!pos_x || !pos_y || !pos_z || !yaw || !energy || !health)
            {
                LOG_WARN("PlayerUpdate from session {} has truncated body", session->id());
                return;
            }

            auto updated = server.world().mutate_player(*pid, [&](PlayerState& state)
            {
                state.pos_x = *pos_x;
                state.pos_y = *pos_y;
                state.pos_z = *pos_z;
                state.rot_yaw = *yaw;
                state.energy = *energy;
                state.health = *health;
            });

            if (!updated)
            {
                LOG_WARN("PlayerUpdate for unknown player {}", *pid);
            }
        }

        void handle_save_game(GameServer& server, Session::Ptr session, const PacketView& packet)
        {
            Reader r(packet.body);
            auto pid = r.read_u64();

            if (!pid || !IsOwner(session, *pid))
            {
                LOG_WARN("SaveGame rejected: session {} is not authenticated as player {}", session->id(), pid ? *pid : 0);
                send_save_response(session, false);
                return;
            }

            const auto chakra = r.read_u32();
            const auto dimension = r.read_u32();
            const auto world_index = r.read_u32();
            const auto karma = r.read_u32();
            const auto reincarnations = r.read_u32();

            if (!chakra || !dimension || !world_index || !karma || !reincarnations)
            {
                LOG_WARN("SaveGame from session {} has truncated body", session->id());
                send_save_response(session, false);
                return;
            }

            auto updated = server.world().mutate_player(*pid, [&](PlayerState& state)
            {
                state.chakra_level = static_cast<ChakraLevel>(*chakra);
                state.dimension = static_cast<DimensionLevel>(*dimension);
                state.world_index = static_cast<WorldIndex>(*world_index);
                state.karma = static_cast<KarmaValue>(*karma);
                state.reincarnation_count = static_cast<i32>(*reincarnations);
                state.last_save_timestamp = NowMillis();
            });

            if (!updated)
            {
                LOG_WARN("SaveGame for unknown player {}", *pid);
                send_save_response(session, false);
                return;
            }

            PlayerRepository repo(server.db_pool());
            auto saved = repo.save(*updated);
            if (!saved)
            {
                LOG_ERROR("SaveGame failed for player {}: {}", *pid, saved.error().message);
                send_save_response(session, false);
                return;
            }

            LOG_INFO("Player {} state saved", *pid);
            send_save_response(session, true);
        }

        void handle_load_game(GameServer& server, Session::Ptr session, const PacketView& packet)
        {
            Reader r(packet.body);
            auto pid = r.read_u64();
            if (!pid || !IsOwner(session, *pid)) return;
            auto local = server.world().get_player_copy(*pid);

            if (!local)
            {
                PlayerRepository repo(server.db_pool());
                auto loaded = repo.load(*pid);

                if (!loaded)
                {
                    LOG_ERROR("LoadGame failed for player {}: {}", *pid, loaded.error().message);
                    send_load_response(session, false);
                    return;
                }

                if (!*loaded)
                {
                    LOG_WARN("LoadGame: no saved state for player {}", *pid);
                    send_load_response(session, false);
                    return;
                }

                local = std::move(**loaded);
                server.world().add_player(*local);
                LOG_INFO("Player {} loaded from Oracle", *pid);
            }

            send_load_response(session, true);
            LogAndContinue("send load state sync", session->send(GameSession(session->id(), session->get_conn(), server).make_state_sync(*local)));
        }
    }

    void PacketHandler::handle(GameServer& server, Session::Ptr session, const PacketView& packet)
    {
        switch (static_cast<PacketType>(packet.type))
        {
            case PacketType::Heartbeat:
                handle_heartbeat(server, std::move(session), packet); break;
            case PacketType::AuthLogin:
                handle_auth_login(server, std::move(session), packet); break;
            case PacketType::PlayerUpdate:
                handle_player_update(server, std::move(session), packet); break;
            case PacketType::SaveGameRequest:
                handle_save_game(server, std::move(session), packet); break;
            case PacketType::LoadGameRequest:
                handle_load_game(server, std::move(session), packet); break;
            default:
                LOG_WARN("Unknown packet type 0x{:04x}", packet.type);
                break;
        }
    }
}
