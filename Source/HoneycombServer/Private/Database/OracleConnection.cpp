#include "OracleConnection.hpp"
#include "Logger.hpp"

namespace Honeycomb::Database
{
    using namespace Honeycomb::Core;

    OracleConnection::~OracleConnection()
    {
        LogAndContinue("Oracle close", close());
    }

    Result<OracleConnection, Error> OracleConnection::open(const OracleConfig& config)
    {
        OracleConnection conn;

        #ifndef HC_HAS_ORACLE_CLIENT
            (void)config;
            return std::unexpected(HC_ERROR(ErrorCode::FeatureDisabled,
                "Oracle client not enabled (define HC_HAS_ORACLE_CLIENT to build with OCI)"));
        #else
            OCIEnv* env = nullptr;
            sword status = OCIEnvCreate(&env, OCI_THREADED | OCI_OBJECT, nullptr,
                nullptr, nullptr, nullptr, 0, nullptr);
            if (status != OCI_SUCCESS)
            {
                return std::unexpected(HC_ERROR(ErrorCode::DatabaseConnectionFailed,
                    "OCIEnvCreate failed"));
            }

            OCIError* err = nullptr;
            status = OCIHandleAlloc(env, (void**)&err, OCI_HTYPE_ERROR, 0, nullptr);
            if (status != OCI_SUCCESS)
            {
                OCIHandleFree(env, OCI_HTYPE_ENV);
                return std::unexpected(HC_ERROR(ErrorCode::DatabaseConnectionFailed,
                    "OCIHandleAlloc(ERROR) failed"));
            }

            OCISvcCtx* svc = nullptr;
            status = OCILogon2(env, err, &svc,
                (const OraText*)config.user.c_str(), config.user.size(),
                (const OraText*)config.password.c_str(), config.password.size(),
                (const OraText*)config.connectString.c_str(), config.connectString.size(),
                OCI_DEFAULT);

            if (status != OCI_SUCCESS)
            {
                text errBuf[512];
                sb4 errCode = 0;
                OCIErrorGet(err, 1, nullptr, &errCode, errBuf, sizeof(errBuf), OCI_HTYPE_ERROR);

                std::string msg = "OCILogon2 failed: ";
                msg += (char*)errBuf;

                OCIHandleFree(err, OCI_HTYPE_ERROR);
                OCIHandleFree(env, OCI_HTYPE_ENV);

                return std::unexpected(HC_ERROR(ErrorCode::DatabaseConnectionFailed, msg));
            }

            conn.env = env;
            conn.err = err;
            conn.svc = svc;
            conn.connected = true;

            return conn;
        #endif
    }

    Result<void, Error> OracleConnection::ping()
    {
        if (!is_open())
        {
            return std::unexpected(HC_ERROR(ErrorCode::DatabaseConnectionFailed, "Not connected"));
        }
        return {};
    }

    Result<void, Error> OracleConnection::close()
    {
        handle_ = nullptr;
        return {};
    }

    Result<void, Error> OracleConnection::execute(std::string_view sql)
    {
        if (!is_open())
        {
            return std::unexpected(HC_ERROR(ErrorCode::DatabaseConnectionFailed, "Not connected"));
        }
        if (logging_)
        {
            LOG_DEBUG("Oracle execute: {}", sql);
        }
        return {};
    }

    Result<void, Error> OracleConnection::execute(std::string_view sql, std::initializer_list<SqlParam> params)
    {
        if (!is_open())
        {
            return std::unexpected(HC_ERROR(ErrorCode::DatabaseConnectionFailed, "Not connected"));
        }

        if (logging_)
        {
            LOG_DEBUG("Oracle execute: {} ({} bind params)", sql, params.size());
        }

        #ifndef HC_HAS_ORACLE_CLIENT
            (void)params;
            return std::unexpected(HC_ERROR(ErrorCode::FeatureDisabled,
                "Oracle client not enabled (define HC_HAS_ORACLE_CLIENT)"));
        #else
            OCIStmt* stmt = nullptr;
            sword status = OCIHandleAlloc(env, (void**)&stmt, OCI_HTYPE_STMT, 0, nullptr);
            if (status != OCI_SUCCESS)
            {
                return std::unexpected(HC_ERROR(ErrorCode::QueryFailed, "OCIHandleAlloc(STMT) failed"));
            }

            auto stmtGuard = std::unique_ptr<OCIStmt, void(*)(OCIStmt*)>(
                stmt, [this](OCIStmt* s) {
                    if (s) OCIHandleFree(s, OCI_HTYPE_STMT);
                }
            );

            status = OCIStmtPrepare(stmt, err, (const OraText*)sql.data(), (ub4)sql.size(), OCI_NTV_SYNTAX, OCI_DEFAULT);
            if (status != OCI_SUCCESS)
            {
                return std::unexpected(HC_ERROR(ErrorCode::QueryFailed,
                    "OCIStmtPrepare failed: " + get_last_error()));
            }

            int bindIndex = 1;
            std::vector<OCIDefine*> dummy;
            for (const auto& param : params)
            {
                OCIBind* bindHandle = nullptr;

                sb2 dty = 0;
                void* valuePtr = nullptr;
                ub2 valueSize = 0;

                std::visit([&](auto&& value) {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, int64_t>) {
                        dty = SQLT_INT;
                        valuePtr = const_cast<int64_t*>(&value);
                        valueSize = sizeof(int64_t);
                    } else if constexpr (std::is_same_v<T, double>) {
                        dty = SQLT_FLT;
                        valuePtr = const_cast<double*>(&value);
                        valueSize = sizeof(double);
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        dty = SQLT_STR;
                        valuePtr = const_cast<char*>(value.c_str());
                        valueSize = (ub2)value.size();
                    } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                        dty = SQLT_STR;
                        valuePtr = nullptr;
                        valueSize = 0;
                    }
                }, param.value);

                status = OCIBindByPos(stmt, &bindHandle, err,
                    bindIndex++,
                    valuePtr, valueSize, dty,
                    nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT);

                if (status != OCI_SUCCESS)
                {
                    return std::unexpected(HC_ERROR(ErrorCode::QueryFailed,
                        "OCIBindByPos failed at index " + std::to_string(bindIndex - 1) +
                        ": " + get_last_error()));
                }
            }

            status = OCIStmtExecute(svc, stmt, err,
                1, 0, nullptr, nullptr, OCI_DEFAULT);
            if (status != OCI_SUCCESS && status != OCI_NO_DATA)
            {
                return std::unexpected(HC_ERROR(ErrorCode::QueryFailed,
                    "OCIStmtExecute failed: " + get_last_error()));
            }

            if (!autocommit_)
            {
                status = OCITransCommit(svc, err, OCI_DEFAULT);
                if (status != OCI_SUCCESS)
                {
                    return std::unexpected(HC_ERROR(ErrorCode::TransactionFailed, "OCITransCommit failed: " + get_last_error()));
                }
            }

            return {};
        #endif
    }

    Result<Vec<Map<String, String>>, Error> OracleConnection::query(std::string_view sql, std::initializer_list<SqlParam> params)
    {
        (void)params;
        if (!is_open())
        {
            return std::unexpected(HC_ERROR(ErrorCode::DatabaseConnectionFailed, "Not connected"));
        }
        if (logging_)
        {
            LOG_DEBUG("Oracle query: {}", sql);
        }
        return Vec<Map<String, String>>{};
    }

    Result<void, Error> OracleConnection::execute_transaction(std::span<const String> statements)
    {
        if (!is_open())
        {
            return std::unexpected(HC_ERROR(ErrorCode::DatabaseConnectionFailed, "Not connected"));
        }
        for (const auto& stmt : statements)
        {
            if (auto err = execute(stmt); !err.has_value())
            {
                LogAndContinue("Oracle rollback", rollback());
                return std::unexpected(err.error());
            }
        }
        return commit();
    }

    Result<void, Error> OracleConnection::commit()
    {
        if (logging_) LOG_DEBUG("Oracle commit");
        return {};
    }

    Result<void, Error> OracleConnection::rollback()
    {
        if (logging_) LOG_DEBUG("Oracle rollback");
        return {};
    }

    OracleConnectionPool::OracleConnectionPool(OracleConfig config)
        : config_(std::move(config))
    {
    }

    Result<OracleConnection*, Error> OracleConnectionPool::acquire()
    {
        std::lock_guard lock(mutex_);

        if (!idle_.empty())
        {
            auto conn = idle_.front();
            idle_.pop_front();
            return conn;
        }

        if (total_ < config_.pool_max)
        {
            auto conn_result = OracleConnection::open(config_);
            if (!conn_result)
            {
                return std::unexpected(conn_result.error());
            }
            auto* conn = new OracleConnection(std::move(*conn_result));
            total_++;
            all_.push_back(conn);
            return conn;
        }

        return std::unexpected(HC_ERROR(ErrorCode::DatabaseConnectionFailed, "Connection pool exhausted"));
    }

    void OracleConnectionPool::release(OracleConnection* conn)
    {
        if (!conn) return;
        std::lock_guard lock(mutex_);
        idle_.push_back(conn);
    }

    void OracleConnectionPool::close_all()
    {
        std::lock_guard lock(mutex_);
        for (auto* conn : all_)
        {
            delete conn;
        }
        all_.clear();
        idle_.clear();
        total_ = 0;
    }
}
