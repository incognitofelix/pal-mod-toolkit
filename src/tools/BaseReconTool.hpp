#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // Reconnaissance tool for the base-camp / worker-assignment systems.
    // Step 1 (here): proves the wiring by anchoring on the player.
    // Later steps: reflection-dump the BaseCamp/worker/inventory classes and hook the
    // native "assign Pal to building" RPC to capture its parameters.
    class BaseReconTool final : public Tool
    {
    public:
        auto name() const -> RC::StringViewType override;
        auto hotkey() const -> RC::Input::Key override;
        auto modifiers() const -> RC::Input::ModifierKeyArray override;
        auto on_activate() -> void override;
    };
}
