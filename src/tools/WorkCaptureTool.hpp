#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // Live-capture tool: while armed (toggle with the hotkey), logs the first occurrence
    // of every UFunction call whose name contains a worker/assignment keyword, together
    // with its context class. Arm it, drop a Pal onto a building (or into the open), and
    // the call chain reveals exactly which RPC performs the assignment.
    class WorkCaptureTool final : public Tool
    {
    public:
        auto name() const -> RC::StringViewType override;
        auto hotkey() const -> RC::Input::Key override;
        auto modifiers() const -> RC::Input::ModifierKeyArray override;
        auto on_activate() -> void override;
    };
}
