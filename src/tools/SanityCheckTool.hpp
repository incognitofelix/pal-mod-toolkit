#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // "pmt sanity" -- verifies every identifier in core/KnownIdentifiers.hpp against the
    // running game and prints PASS/FAIL/SKIP. Run it after every Palworld patch (in a
    // loaded world, standing in your base, for full coverage): a FAIL pinpoints exactly
    // which renamed class/function/param broke a tool, instead of tools failing silently.
    class SanityCheckTool final : public Tool
    {
    public:
        auto command() const -> RC::StringViewType override;
        auto help() const -> RC::StringViewType override;
        auto execute(const std::vector<RC::StringType>& args, Out& out) -> void override;
    };
}
