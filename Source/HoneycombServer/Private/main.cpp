#include "Core.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include <csignal>

using namespace Honeycomb;
using namespace Honeycomb::Game;

namespace
{
    std::atomic<bool> g_stop{false};

    void signal_handler(int)
    {
        g_stop = true;
    }
}

[[nodiscard]] Opt<ServerConfig> load_config(int argc, char** argv)
{
    ServerConfig config;

    for (int i = 1; i < argc; ++i)
    {
        String arg = argv[i];
        auto next = [&](const char* name) -> Opt<String> {
            if (i + 1 < argc) return String{argv[++i]};
            LOG_WARN("Missing value for {}", name);
            return std::nullopt;
        };

        if (arg == "--host" || arg == "-h") {
            if (auto v = next("host")) config.bind_address = *v;
        }
        else if (arg == "--tcp-port") {
            if (auto v = next("tcp")) config.tcp_port = static_cast<u16>(std::stoi(*v));
        }
        else if (arg == "--udp-port") {
            if (auto v = next("udp")) config.udp_port = static_cast<u16>(std::stoi(*v));
        }
        else if (arg == "--http-port") {
            if (auto v = next("http")) config.http_port = static_cast<u16>(std::stoi(*v));
        }
        else if (arg == "--max-players") {
            if (auto v = next("max_players")) config.max_players = std::stoi(*v);
        }
        else if (arg == "--db-user") {
            if (auto v = next("db_user")) config.database.user = *v;
        }
        else if (arg == "--db-password") {
            if (auto v = next("db_password")) config.database.password = *v;
        }
        else if (arg == "--db-host") {
            if (auto v = next("db_host")) config.database.host = *v;
        }
        else if (arg == "--config" || arg == "-c") {
            if (auto v = next("config")) {
                LOG_WARN("Config file '{}' — пока не поддерживается, используйте флаги", *v);
            }
        }
        else if (arg == "--help" || arg == "/?") {
            std::println("Honeycomb Server\n"
                         "  --host <ip>          bind address (default 0.0.0.0)\n"
                         "  --tcp-port <n>       game TCP port (default 7777)\n"
                         "  --udp-port <n>       game UDP port (default 7778)\n"
                         "  --http-port <n>      HTTP API port (default 8080)\n"
                         "  --max-players <n>    max online players\n"
                         "  --db-user/password/host  Oracle connection\n");
            return std::nullopt;
        }
        else {
            LOG_WARN("Unknown argument: {}", arg);
        }
    }

    return config;
}

int HoneycombServerMain(int argc, char** argv)
{
    LOG_INFO("Honeycomb Server (C++26) starting...");

    auto config = load_config(argc, argv);
    if (!config)
    {
        return 1;
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    GameServer server(*config);
    auto result = server.start();
    if (!result)
    {
        LOG_FATAL("Server failed to start: {}", result.error().message);
        return 1;
    }

    LOG_INFO("Honeycomb server is running. Press Ctrl+C to stop.");

    while (!g_stop)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        server.world().tick(0.1f);
    }

    LogAndContinue("server stop", server.stop());
    LOG_INFO("Honeycomb server stopped. Goodbye!");
    return 0;
}
