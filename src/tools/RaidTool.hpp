#pragma once

#include "core/Tool.hpp"

namespace PMT
{
    // "pmt raid" -- triggers a base-raid invader incident on the base you are standing in
    // (PalInvaderManager::RequestIncidentInvaderEnemy_BP). Useful for testing raid behaviour
    // on demand (e.g. the PalInvaderRelocator).
    class RaidTool final : public Tool
    {
    public:
        auto command() const -> RC::StringViewType override;
        auto help() const -> RC::StringViewType override;
        auto execute(const std::vector<RC::StringType>& args, Out& out) -> void override;

    private:
        // Last incident we created, so "pmt raid clear" can remove it (a still-active
        // incident blocks a fresh raid request).
        RC::Unreal::UObject* m_last_incident = nullptr;
    };
}
