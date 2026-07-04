// Shared.hpp first: see the note in PlayerLocationTool.cpp.
#include "Shared.hpp"
#include "core/Console.hpp"
#include "core/KnownIdentifiers.hpp"
#include "core/Player.hpp"
#include "tools/NearbyActorsTool.hpp"

#include <vector>
#include <string>
#include <cmath>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    auto NearbyActorsTool::command() const -> StringViewType { return STR("actors"); }
    auto NearbyActorsTool::help() const -> StringViewType
    {
        return STR("list Actors within [meters] of the player (default 50). Details -> UE4SS.log");
    }

    auto NearbyActorsTool::execute(const std::vector<StringType>& args, Out& out) -> void
    {
        double radius_cm = 5000.0; // 50 m default
        if (!args.empty())
        {
            try { radius_cm = std::stod(args[0]) * 100.0; }
            catch (...) { say(out, STR("bad radius '{}', using 50m"), args[0]); }
        }

        auto* player = current_player(out);
        if (!player) { say(out, STR("no player found")); return; }
        const FVector origin = player->K2_GetActorLocation();

        std::vector<UObject*> actors;
        UObjectGlobals::FindAllOf(Identifiers::Actor, actors);

        const double radius_sq = radius_cm * radius_cm;
        int n = 0;
        Output::send<LogLevel::Warning>(STR("[pmt] === Actors within {}m ===\n"), radius_cm / 100.0);
        for (auto* obj : actors)
        {
            const FVector p = static_cast<AActor*>(obj)->K2_GetActorLocation();
            const double dx = p.X() - origin.X();
            const double dy = p.Y() - origin.Y();
            const double dz = p.Z() - origin.Z();
            const double dist_sq = dx * dx + dy * dy + dz * dz;
            if (dist_sq <= radius_sq)
            {
                Output::send<LogLevel::Warning>(STR("[pmt]  {}m  {}\n"),
                                                std::sqrt(dist_sq) / 100.0, obj->GetFullName());
                ++n;
            }
        }
        say(out, STR("{} actor(s) within {}m (details in UE4SS.log)"), n, radius_cm / 100.0);
    }
}
