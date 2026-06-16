// Shared.hpp first: see the note in PlayerLocationTool.cpp.
#include "Shared.hpp"
#include "tools/BaseReconTool.hpp"

#include <Unreal/UClass.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/FProperty.hpp>

#include <array>
#include <vector>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    namespace
    {
        // Names whose appearance in a property/function we want flagged in the log, so the
        // navigation path to the worker-assignment system jumps out of a big dump.
        const std::array<StringType, 9> k_keywords = {
            STR("BaseCamp"), STR("Worker"), STR("Assign"), STR("Reserve"),
            STR("Suitability"), STR("Individual"), STR("MapObject"),
            STR("Inventory"), STR("Work"),
        };

        // Where to stop walking up the inheritance chain: engine base classes carry no
        // Pal-specific info and would bloat the dump enormously.
        const std::array<StringType, 6> k_stop = {
            STR("Actor"), STR("Object"), STR("Pawn"),
            STR("Character"), STR("ActorComponent"), STR("SceneComponent"),
        };

        auto contains(const std::array<StringType, 6>& set, const StringType& name) -> bool
        {
            for (const auto& s : set) { if (s == name) { return true; } }
            return false;
        }

        auto has_keyword(const StringType& text) -> bool
        {
            for (const auto& kw : k_keywords)
            {
                if (text.find(kw) != StringType::npos) { return true; }
            }
            return false;
        }

        // MSVC C4996: ForEachProperty/ForEachFunction are deprecated in favour of TFieldRange,
        // but the deprecated members still return a TFieldRange and are the simplest way to
        // iterate a single inheritance level. Silence the warning locally.
#pragma warning(push)
#pragma warning(disable : 4996)

        // Dumps one inheritance level (this struct's own properties + functions).
        auto dump_struct(UStruct* s) -> void
        {
            Output::send<LogLevel::Warning>(STR("[Recon] ==== {} ====\n"), s->GetFullName());

            for (FProperty* prop : s->ForEachProperty())
            {
                const StringType pname = prop->GetName();
                const StringType ptype = prop->GetClass().GetName();
                Output::send<LogLevel::Warning>(STR("[Recon]   prop {} : {}\n"), pname, ptype);
                if (has_keyword(pname))
                {
                    Output::send<LogLevel::Warning>(STR("[Recon]   *** HIT prop {}\n"), pname);
                }
            }

            for (UFunction* fn : s->ForEachFunction())
            {
                StringType sig{};
                for (FProperty* param : fn->ForEachProperty())
                {
                    if (!sig.empty()) { sig += STR(", "); }
                    sig += param->GetName();
                    sig += STR(":");
                    sig += param->GetClass().GetName();
                }
                const StringType fname = fn->GetName();
                Output::send<LogLevel::Warning>(STR("[Recon]   fn {}({})\n"), fname, sig);
                if (has_keyword(fname) || has_keyword(sig))
                {
                    Output::send<LogLevel::Warning>(STR("[Recon]   *** HIT fn {}\n"), fname);
                }
            }
        }

#pragma warning(pop)

        // Dumps a class and walks up its native super chain until a STOP class is hit.
        auto walk(UStruct* cls) -> void
        {
            UStruct* cur = cls;
            for (int guard = 0; cur && guard < 16; ++guard)
            {
                dump_struct(cur);
                if (contains(k_stop, cur->GetName())) { break; }
                cur = cur->GetSuperStruct();
            }
        }
    }

    auto BaseReconTool::name() const -> StringViewType
    {
        return STR("Base Recon");
    }

    auto BaseReconTool::hotkey() const -> Input::Key
    {
        return Input::Key::F3;
    }

    auto BaseReconTool::modifiers() const -> Input::ModifierKeyArray
    {
        // Shift + F3, consistent with F1/F2 (see PlayerLocationTool).
        return { Input::ModifierKey::SHIFT };
    }

    auto BaseReconTool::on_activate() -> void
    {
        Output::send<LogLevel::Warning>(STR("[Recon] === Base Recon: Dump START ===\n"));

        // Seed classes: the player-side classes (proven anchor) plus two hopeful guesses for
        // the base-camp class. The dump + HIT markers reveal how to navigate to the
        // worker-assignment system; found names become seeds in the next step.
        const std::array<const TCHAR*, 4> seeds = {
            // Last lead: does the Pal's base-camp AI (or its worker module) expose a direct
            // "force-assign to this WorkId/WorkProgressId" INPUT? If yes -> clean per-building
            // assignment. If only the distance-based SetBaseCampActionWithFixAssign exists,
            // assignment is inherently position-based.
            STR("/Game/Pal/Blueprint/Controller/Monster/BP_MonsterAIController_BaseCamp.BP_MonsterAIController_BaseCamp_C"),
            STR("/Script/Pal.PalActionBase"),
            STR("/Script/Pal.PalMonsterAIControllerBase"),
            STR("/Script/Pal.PalCharacterParameterComponent"),
        };

        for (const TCHAR* path : seeds)
        {
            auto* cls = UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, path);
            if (!cls)
            {
                Output::send<LogLevel::Warning>(STR("[Recon] [NOT FOUND] {}\n"), path);
                continue;
            }
            walk(cls);
        }

        Output::send<LogLevel::Warning>(STR("[Recon] === Base Recon: dump END (search '*** HIT') ===\n"));
    }
}
