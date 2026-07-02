#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // "pmt defaultpos" -- behavioural probe: calls SetDefaultPositionAction() on every
    // worker Pal's controller (they walk to their default spot). Temporary diagnostic.
    class BaseActionTestTool final : public Tool
    {
    public:
        auto command() const -> RC::StringViewType override;
        auto help() const -> RC::StringViewType override;
        auto execute(const std::vector<RC::StringType>& args, Out& out) -> void override;
    };
}
