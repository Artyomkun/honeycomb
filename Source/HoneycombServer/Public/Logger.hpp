#pragma once

#include "Core.hpp"

#pragma push_macro("verify")
#pragma push_macro("check")
#undef verify
#undef check

#include "absl/log/log.h"
#include "absl/log/absl_log.h"
#include "absl/log/initialize.h"
#include "absl/log/globals.h"
#include "absl/strings/str_format.h"

#pragma pop_macro("check")
#pragma pop_macro("verify")

namespace Honeycomb::Core
{
    inline void InitializeLogger()
    {
        absl::InitializeLog();
        absl::SetStderrThreshold(absl::LogSeverityAtLeast::kInfo);
    }

    inline void SetLogLevel(int severity)
    {
        absl::SetStderrThreshold(static_cast<absl::LogSeverityAtLeast>(severity));
    }
}

#define LOG_TRACE(...)   ABSL_LOG(LEVEL(0)) << absl::StrFormat(__VA_ARGS__)
#define LOG_DEBUG(...)   ABSL_LOG(LEVEL(1)) << absl::StrFormat(__VA_ARGS__)
#define LOG_INFO(...)    ABSL_LOG(INFO)      << absl::StrFormat(__VA_ARGS__)
#define LOG_WARNING(...) ABSL_LOG(WARNING)   << absl::StrFormat(__VA_ARGS__)
#define LOG_WARN(...)    ABSL_LOG(WARNING)   << absl::StrFormat(__VA_ARGS__)
#define LOG_ERROR(...)   ABSL_LOG(ERROR)     << absl::StrFormat(__VA_ARGS__)
#define LOG_FATAL(...)   ABSL_LOG(FATAL)     << absl::StrFormat(__VA_ARGS__)

namespace Honeycomb
{
    inline void LogAndContinue(std::string_view What, const Core::Result<void, Core::Error>& Result)
    {
        if (!Result)
        {
            LOG_WARN("{} failed: {}", What, Result.error().message);
        }
    }
}
