#include "GameSession.hpp"
#include "Server.hpp"
#include "Logger.hpp"

namespace Honeycomb::Game
{
    namespace Protocol
    {
        void Writer::write_u8(u8 v) { buffer_.push_back(v); }

        void Writer::write_u16(u16 v)
        {
            buffer_.push_back(static_cast<u8>(v & 0xFF));
            buffer_.push_back(static_cast<u8>((v >> 8) & 0xFF));
        }

        void Writer::write_u32(u32 v)
        {
            for (int i = 0; i < 4; ++i)
            buffer_.push_back(static_cast<u8>((v >> (i * 8)) & 0xFF));
        }

        void Writer::write_u64(u64 v)
        {
            for (int i = 0; i < 8; ++i)
            buffer_.push_back(static_cast<u8>((v >> (i * 8)) & 0xFF));
        }

        void Writer::write_f32(f32 v)
        {
            u32 bits;
            std::memcpy(&bits, &v, 4);
            write_u32(bits);
        }

        void Writer::write_f64(f64 v)
        {
            u64 bits;
            std::memcpy(&bits, &v, 8);
            write_u64(bits);
        }

        void Writer::string(std::string_view s)
        {
            write_u32(static_cast<u32>(s.size()));
            buffer_.insert(buffer_.end(), s.begin(), s.end());
        }

        Result<u8, Error> Reader::read_u8()
        {
            if (remaining() < 1) return std::unexpected(HC_ERROR(ErrorCode::SerializationError, "u8 overflow"));
            return data_[pos_++];
        }

        Result<u16, Error> Reader::read_u16()
        {
            if (remaining() < 2) return std::unexpected(HC_ERROR(ErrorCode::SerializationError, "u16 overflow"));
            u16 v = data_[pos_] | (data_[pos_ + 1] << 8);
            pos_ += 2;
            return v;
        }

        Result<u32, Error> Reader::read_u32()
        {
            if (remaining() < 4) return std::unexpected(HC_ERROR(ErrorCode::SerializationError, "u32 overflow"));
            u32 v = 0;
            for (int i = 0; i < 4; ++i) v |= static_cast<u32>(data_[pos_ + i]) << (i * 8);
            pos_ += 4;
            return v;
        }

        Result<u64, Error> Reader::read_u64()
        {
            if (remaining() < 8) return std::unexpected(HC_ERROR(ErrorCode::SerializationError, "u64 overflow"));
            u64 v = 0;
            for (int i = 0; i < 8; ++i) v |= static_cast<u64>(data_[pos_ + i]) << (i * 8);
            pos_ += 8;
            return v;
        }

        Result<f32, Error> Reader::read_f32()
        {
            auto bits = read_u32();
            if (!bits) return std::unexpected(bits.error());
            f32 v;
            std::memcpy(&v, &*bits, 4);
            return v;
        }

        Result<f64, Error> Reader::read_f64()
        {
            auto bits = read_u64();
            if (!bits) return std::unexpected(bits.error());
            f64 v;
            std::memcpy(&v, &*bits, 8);
            return v;
        }

        Result<StringView, Error> Reader::string_view()
        {
            auto len_opt = read_u32();
            if (!len_opt) return std::unexpected(len_opt.error());
            u32 len = *len_opt;
            if (remaining() < len) return std::unexpected(HC_ERROR(ErrorCode::SerializationError, "string overflow"));
            StringView sv(reinterpret_cast<const char*>(data_.data() + pos_), len);
            pos_ += len;
            return sv;
        }
    }

    GameSession::GameSession(SessionId id, Network::FSocketRef socket, GameServer& server): id_(id), socket_(std::move(socket)), server_(server), player_id_(std::nullopt), active_(true)
    {
        LOG_DEBUG("GameSession {}: created", id_);
    }

    void GameSession::on_player_joined(Session::Ptr session)
    {
        LOG_INFO("Player joined session {}", session->id());
    }

    void GameSession::on_player_left(PlayerId pid)
    {
        LOG_INFO("Player {} left", pid);
    }

    Result<void, Error> GameSession::save_player_state(SessionId id)
    {
        (void)id;
        return {};
    }

    Result<void, Error> GameSession::load_player_state(SessionId id)
    {
        (void)id;
        return {};
    }

    Vec<u8> GameSession::make_auth_response(bool success, std::string_view message, PlayerId pid)
    {
        Protocol::Writer w;
        w.write_u16(static_cast<u16>(PacketType::AuthLogin));
        w.write_u8(success ? 1 : 0);
        w.string(message);
        w.write_u64(pid);
        return std::move(w).bytes();
    }

    Vec<u8> GameSession::make_state_sync(const PlayerState& state)
    {
        Protocol::Writer w;
        w.write_u16(static_cast<u16>(PacketType::PlayerStateSync));
        w.write_u64(state.id);
        w.write_u32(static_cast<u32>(state.chakra_level));
        w.write_u32(static_cast<u32>(state.dimension));
        w.write_u32(static_cast<u32>(state.world_index));
        w.write_u32(static_cast<u32>(state.karma));
        w.write_u32(static_cast<u32>(state.reincarnation_count));
        w.write_f32(state.energy);
        w.write_f32(state.max_energy);
        w.write_f32(state.health);
        w.write_f32(state.pos_x);
        w.write_f32(state.pos_y);
        w.write_f32(state.pos_z);
        w.write_f32(state.rot_yaw);
        w.write_u8(state.is_dead ? 1 : 0);
        return std::move(w).bytes();
    }
}