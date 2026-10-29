#pragma once
#include "../Core.hpp"
#include <deque>

namespace Honeycomb::Database
{
    using namespace Honeycomb::Core;

    /** Значение bind-параметра (:1, :2, ...). В текст SQL значения не подставляются. */
    using SqlParam = std::variant<std::string, i64, double>;

    struct OracleConfig
    {
        String host = "127.0.0.1";
        u16 port = 1521;
        String service_name = "ORCL";
        String user;
        String password;
        i32 pool_min = 1;
        i32 pool_max = 8;
        i32 connection_timeout_ms = 10000;
        bool log_queries = false;
    };

    class OracleConnection
    {
    public:
        OracleConnection() = default;
        ~OracleConnection();

        OracleConnection(const OracleConnection&) = delete;
        OracleConnection& operator=(const OracleConnection&) = delete;
        OracleConnection(OracleConnection&&) noexcept = default;
        OracleConnection& operator=(OracleConnection&&) noexcept = default;

        [[nodiscard]] static Result<OracleConnection, Error> open(const OracleConfig& config);

        Result<void, Error> ping();
        Result<void, Error> close();

        [[nodiscard]] bool is_open() const noexcept { return handle_ != nullptr; }

        Result<void, Error> execute(std::string_view sql);

        // Выполнение DDL/DML с bind-параметрами (:1, :2, ...) — без конкатенации SQL
        Result<void, Error> execute(std::string_view sql, std::initializer_list<SqlParam> params);

        [[nodiscard]] Result<Vec<Map<String, String>>, Error> query(
            std::string_view sql,
            std::initializer_list<SqlParam> params = {});

        Result<void, Error> execute_transaction(std::span<const String> statements);

    private:
        void* handle_ = nullptr;
        void* err_handle_ = nullptr;
        void* svc_handle_ = nullptr;
        bool logging_ = false;

        Result<void, Error> commit();
        Result<void, Error> rollback();
    };

    struct OracleConnectionPool
    {
    public:
        explicit OracleConnectionPool(OracleConfig config);

        [[nodiscard]] Result<OracleConnection*, Error> acquire();
        void release(OracleConnection* conn);
        void close_all();

        [[nodiscard]] usize idle_count() const noexcept { return idle_.size(); }
        [[nodiscard]] usize total_count() const noexcept { return total_; }

    private:
        OracleConfig config_;
        std::deque<OracleConnection*> idle_;
        Vec<OracleConnection*> all_;
        std::mutex mutex_;
        usize total_ = 0;
        bool closed_ = false;
    };
}