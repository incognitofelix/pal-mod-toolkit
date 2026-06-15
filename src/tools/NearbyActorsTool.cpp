// Shared.hpp first: see the note in PlayerLocationTool.cpp.
#include "Shared.hpp"
#include "tools/NearbyActorsTool.hpp"

#include <vector>
#include <cmath>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    auto NearbyActorsTool::name() const -> StringViewType
    {
        return STR("Nearby Actors");
    }

    auto NearbyActorsTool::hotkey() const -> Input::Key
    {
        return Input::Key::F2;
    }

    auto NearbyActorsTool::modifiers() const -> Input::ModifierKeyArray
    {
        // Shift + F2 (see PlayerLocationTool for why Shift over Ctrl/Alt).
        return { Input::ModifierKey::SHIFT };
    }

    auto NearbyActorsTool::on_activate() -> void
    {
        // 1) Anchor: the player's position.
        auto* player = UObjectGlobals::FindFirstOf(STR("PalPlayerCharacter"));
        if (!player)
        {
            Output::send<LogLevel::Error>(STR("[PalModToolkit] No PalPlayerCharacter found\n"));
            return;
        }
        const FVector origin = static_cast<AActor*>(player)->K2_GetActorLocation();

        // 2) Every actor currently loaded.
        std::vector<UObject*> actors;
        UObjectGlobals::FindAllOf(STR("Actor"), actors);

        // 3) Log the ones inside the radius. We compare squared distances (no sqrt) for
        //    speed, and only take the real sqrt for the few that actually match.
        constexpr double radius_sq = s_radius_cm * s_radius_cm;

        Output::send<LogLevel::Warning>(STR("[PalModToolkit] === Actors within {}m ===\n"),
                                        s_radius_cm / 100.0);
        for (auto* obj : actors)
        {
            const FVector p = static_cast<AActor*>(obj)->K2_GetActorLocation();
            const double dx = p.X() - origin.X();
            const double dy = p.Y() - origin.Y();
            const double dz = p.Z() - origin.Z();
            const double dist_sq = dx * dx + dy * dy + dz * dz;

            if (dist_sq <= radius_sq)
            {
                Output::send<LogLevel::Warning>(STR("[PalModToolkit]  {}m  {}\n"),
                                                std::sqrt(dist_sq) / 100.0, obj->GetFullName());
            }
        }
        Output::send<LogLevel::Warning>(STR("[PalModToolkit] === end ===\n"));
    }
}
