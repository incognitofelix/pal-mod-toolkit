// Shared.hpp first: it pulls in the global UE4SS config (CharType, RC_IS_ANSI, ...)
// that DynamicOutput's formatting macros rely on.
#include "Shared.hpp"
#include "core/Console.hpp"
#include "core/Player.hpp"
#include "tools/PlayerLocationTool.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    auto PlayerLocationTool::command() const -> StringViewType { return STR("loc"); }
    auto PlayerLocationTool::help() const -> StringViewType
    {
        return STR("print the player's world position (X/Y/Z, cm)");
    }

    auto PlayerLocationTool::execute(const std::vector<StringType>&, Out& out) -> void
    {
        auto* player = current_player(out);
        if (!player) { say(out, STR("no player found")); return; }
        const FVector loc = player->K2_GetActorLocation();
        say(out, STR("player pos: X={} Y={} Z={}"), loc.X(), loc.Y(), loc.Z());
    }
}
