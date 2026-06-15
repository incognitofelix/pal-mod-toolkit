// Shared.hpp first: it pulls in the global UE4SS config (CharType, RC_IS_ANSI, ...)
// that DynamicOutput's formatting macros rely on. Including a lighter header first
// breaks Output.hpp's macro expansion.
#include "Shared.hpp"
#include "tools/PlayerLocationTool.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    auto PlayerLocationTool::name() const -> StringViewType
    {
        return STR("Player Location");
    }

    auto PlayerLocationTool::hotkey() const -> Input::Key
    {
        return Input::Key::F2;
    }

    auto PlayerLocationTool::on_activate() -> void
    {
        auto* player = UObjectGlobals::FindFirstOf(STR("PalPlayerCharacter"));
        if (!player)
        {
            Output::send<LogLevel::Error>(STR("[PalModToolkit] No PalPlayerCharacter found\n"));
            return;
        }

        FVector loc = static_cast<AActor*>(player)->K2_GetActorLocation();
        Output::send<LogLevel::Warning>(STR("[PalModToolkit] Player pos: X={} Y={} Z={}\n"),
                                        loc.X(), loc.Y(), loc.Z());
    }
}
