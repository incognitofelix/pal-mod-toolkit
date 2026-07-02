#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // "pmt works" -- lists the base's work objects (PalWorkBase) near the player, with
    // their world position. Foundation for assigning a Pal to a chosen building.
    class ListWorksTool final : public Tool
    {
    public:
        auto command() const -> RC::StringViewType override;
        auto help() const -> RC::StringViewType override;
        auto execute(const std::vector<RC::StringType>& args, Out& out) -> void override;
    };
}
