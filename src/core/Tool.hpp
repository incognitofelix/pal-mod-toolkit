#pragma once

#include <String/StringType.hpp>
#include <vector>

namespace RC::Unreal { class FOutputDevice; class UObject; }

namespace PMT
{
    // Context for one command invocation.
    //  - console: in-game console output device (null for chat-driven commands).
    //  - player:  the object that issued the command (console Executor or chat Context,
    //             a controller/character/component). Resolved via current_player() so
    //             position/distance commands use the RIGHT player in multiplayer/server.
    struct Out
    {
        RC::Unreal::FOutputDevice* console = nullptr;
        RC::Unreal::UObject* player = nullptr;
    };

    // Abstract base class for every toolkit tool.
    //
    // A "tool" is one command, invoked as "pmt <command> [args]" either from the in-game
    // console or from chat. To add a tool:
    //   1. Create a class deriving from Tool and implement the three methods below.
    //   2. Register it once in PalModToolkit's constructor.
    class Tool
    {
    public:
        virtual ~Tool() = default;

        // The subcommand word, e.g. STR("loc") -> invoked as "pmt loc".
        virtual auto command() const -> RC::StringViewType = 0;

        // One-line usage shown by "pmt help".
        virtual auto help() const -> RC::StringViewType = 0;

        // Runs the tool. `args` are the tokens after the subcommand. Use the say() helper
        // with `out` to write to the console (when present) and the log.
        virtual auto execute(const std::vector<RC::StringType>& args, Out& out) -> void = 0;
    };
}
