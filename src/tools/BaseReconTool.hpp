#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // "pmt recon [Class ...]" -- reflection-dumps the given classes (properties +
    // functions, walking the native super chain) to the UE4SS log, flagging keyword
    // HITs. A bare name is resolved as /Script/Pal.<Name>; pass a full path otherwise.
    class BaseReconTool final : public Tool
    {
    public:
        auto command() const -> RC::StringViewType override;
        auto help() const -> RC::StringViewType override;
        auto execute(const std::vector<RC::StringType>& args, Out& out) -> void override;
    };
}
