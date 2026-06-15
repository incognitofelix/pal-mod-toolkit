#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // Prints the local player's world position (X/Y/Z, in cm) to the console.
    class PlayerLocationTool final : public Tool
    {
    public:
        auto name() const -> RC::StringViewType override;
        auto hotkey() const -> RC::Input::Key override;
        auto modifiers() const -> RC::Input::ModifierKeyArray override;
        auto on_activate() -> void override;
    };
}
