#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // "pmt capture" -- toggle. While armed, logs the first occurrence of every UFunction
    // call whose name contains a worker/assignment keyword, with parameter values, to
    // reveal the native call chain. Arm it, perform the action, run "pmt capture" again.
    class WorkCaptureTool final : public Tool
    {
    public:
        auto command() const -> RC::StringViewType override;
        auto help() const -> RC::StringViewType override;
        auto execute(const std::vector<RC::StringType>& args, Out& out) -> void override;
    };
}
