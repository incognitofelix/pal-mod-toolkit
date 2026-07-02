#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // "pmt assign" -- proven assignment backend probe: logs the first base Pal's current
    // work, then teleports it to the player and fix-assigns it (native V-drop logic).
    // Run it, then run "pmt assign" again at a building to verify currentWork changed.
    class AssignTestTool final : public Tool
    {
    public:
        auto command() const -> RC::StringViewType override;
        auto help() const -> RC::StringViewType override;
        auto execute(const std::vector<RC::StringType>& args, Out& out) -> void override;
    };
}
