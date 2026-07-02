#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // "pmt loc" -- prints the local player's world position (X/Y/Z, in cm).
    class PlayerLocationTool final : public Tool
    {
    public:
        auto command() const -> RC::StringViewType override;
        auto help() const -> RC::StringViewType override;
        auto execute(const std::vector<RC::StringType>& args, Out& out) -> void override;
    };
}
