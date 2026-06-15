#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // Lists every Actor within a radius of the player (distance + full name/class).
    // Primary use: reconnaissance — finding spawners, dungeons, map objects at a spot.
    class NearbyActorsTool final : public Tool
    {
    public:
        auto name() const -> RC::StringViewType override;
        auto hotkey() const -> RC::Input::Key override;
        auto modifiers() const -> RC::Input::ModifierKeyArray override;
        auto on_activate() -> void override;

    private:
        // Search radius around the player, in centimeters (Unreal world units).
        static constexpr double s_radius_cm = 5000.0;
    };
}
