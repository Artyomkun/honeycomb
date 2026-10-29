#pragma once

#include "Core.hpp"
#include "Sockets.h"
#include "SocketSubsystem.h"

namespace Honeycomb::Network
{
    using FSocketRef = TSharedPtr<FSocket>;

    [[nodiscard]] inline FSocketRef MakeSocketRef(FSocket* RawSocket, ISocketSubsystem* Subsystem = nullptr)
    {
        if (RawSocket == nullptr)
        {
            return nullptr;
        }

        if (Subsystem == nullptr)
        {
            Subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
        }

        return MakeShareable(RawSocket, FSocketDeleter(Subsystem));
    }
}
