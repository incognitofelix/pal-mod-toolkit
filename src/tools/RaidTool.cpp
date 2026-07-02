// Shared.hpp first: see the note in PlayerLocationTool.cpp.
#include "Shared.hpp"
#include "core/Console.hpp"
#include "core/Player.hpp"
#include "tools/RaidTool.hpp"

#include <Unreal/UFunction.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UnrealFlags.hpp>

#include <vector>
#include <cstdint>
#include <cstring>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    namespace
    {
#pragma warning(push)
#pragma warning(disable : 4996) // ForEachProperty deprecated; simplest iterator.

        // The base-camp model the player is standing in (PalBaseCampManager::GetNearestBaseCamp).
        auto get_player_base(const FVector& loc) -> UObject*
        {
            auto* mgr = UObjectGlobals::FindFirstOf(STR("PalBaseCampManager"));
            if (!mgr) { return nullptr; }
            auto* fn = mgr->GetFunctionByNameInChain(STR("GetNearestBaseCamp"));
            if (!fn) { return nullptr; }

            std::vector<uint8_t> frame(static_cast<size_t>(fn->GetStructureSize()), 0);
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->GetName() == STR("Location"))
                {
                    std::memcpy(frame.data() + p->GetOffset_Internal(), &loc, sizeof(FVector));
                }
            }
            mgr->ProcessEvent(fn, frame.data());
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->HasAnyPropertyFlags(CPF_ReturnParm))
                {
                    return *reinterpret_cast<UObject* const*>(frame.data() + p->GetOffset_Internal());
                }
            }
            return nullptr;
        }

        // PalInvaderManager::RemoveInvaderIncident(Incident) -> clears an active incident.
        auto remove_incident(UObject* manager, UObject* incident) -> void
        {
            auto* fn = manager->GetFunctionByNameInChain(STR("RemoveInvaderIncident"));
            if (!fn) { return; }
            std::vector<uint8_t> frame(static_cast<size_t>(fn->GetStructureSize()), 0);
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->GetName() == STR("Incident"))
                {
                    *reinterpret_cast<UObject**>(frame.data() + p->GetOffset_Internal()) = incident;
                }
            }
            manager->ProcessEvent(fn, frame.data());
        }

        // PalInvaderManager::StartInvaderMarchAll() -- actually starts registered invaders.
        auto start_march(UObject* manager) -> bool
        {
            auto* fn = manager->GetFunctionByNameInChain(STR("StartInvaderMarchAll"));
            if (!fn) { return false; }
            manager->ProcessEvent(fn, nullptr);
            return true;
        }

        // PalInvaderManager::RequestIncidentInvaderEnemy_BP(base, Parameter=null) -> incident.
        auto request_raid(UObject* manager, UObject* base_camp) -> UObject*
        {
            auto* fn = manager->GetFunctionByNameInChain(STR("RequestIncidentInvaderEnemy_BP"));
            if (!fn) { return nullptr; }

            std::vector<uint8_t> frame(static_cast<size_t>(fn->GetStructureSize()), 0);
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->GetName() == STR("OccuredBaseCamp"))
                {
                    *reinterpret_cast<UObject**>(frame.data() + p->GetOffset_Internal()) = base_camp;
                }
                // Parameter is left null -> the game uses a default invasion.
            }
            manager->ProcessEvent(fn, frame.data());
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->HasAnyPropertyFlags(CPF_ReturnParm))
                {
                    return *reinterpret_cast<UObject* const*>(frame.data() + p->GetOffset_Internal());
                }
            }
            return nullptr;
        }

#pragma warning(pop)
    }

    auto RaidTool::command() const -> StringViewType { return STR("raid"); }
    auto RaidTool::help() const -> StringViewType
    {
        return STR("trigger a base raid on your current base. 'pmt raid clear' removes the last incident");
    }

    auto RaidTool::execute(const std::vector<StringType>& args, Out& out) -> void
    {
        auto* manager = UObjectGlobals::FindFirstOf(STR("PalInvaderManager"));
        if (!manager) { say(out, STR("no PalInvaderManager found")); return; }

        // "pmt raid clear" -> remove the incident we last created (frees a new request).
        if (!args.empty() && args[0] == STR("clear"))
        {
            if (m_last_incident) { remove_incident(manager, m_last_incident); m_last_incident = nullptr; say(out, STR("cleared last incident")); }
            else { say(out, STR("no incident to clear")); }
            return;
        }

        auto* player = current_player(out);
        if (!player) { say(out, STR("no player found")); return; }
        const FVector loc = player->K2_GetActorLocation();

        auto* base = get_player_base(loc);
        if (!base) { say(out, STR("no base camp at your location")); return; }

        auto* incident = request_raid(manager, base);
        m_last_incident = incident;
        start_march(manager);
        say(out, STR("raid on base '{}' (you at X={} Y={}) -> incident={}"),
            base->GetName(), loc.X(), loc.Y(),
            incident ? incident->GetName() : StringType(STR("(null)")));
    }
}
