// Shared.hpp first: see the note in PlayerLocationTool.cpp.
#include "Shared.hpp"
#include "core/Console.hpp"
#include "core/KnownIdentifiers.hpp"
#include "tools/SanityCheckTool.hpp"

#include <Unreal/UClass.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/FProperty.hpp>

#include <array>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    namespace
    {
        namespace ID = Identifiers;

        // One row = one thing that a game patch could rename. Table-driven so adding a
        // check is one line, and the check logic exists exactly once per kind.
        enum class Kind
        {
            ScriptClass,   // target = full /Script/ path      -> StaticFindObject
            Instance,      // target = class name              -> FindFirstOf
            Function,      // target = class, member = UFunction name
            FunctionParam, // target = class, member = UFunction, param = parameter name
            Property,      // target = class, member = FProperty name
        };

        struct Check
        {
            Kind kind;
            const TCHAR* target;
            const TCHAR* member = nullptr;
            const TCHAR* param = nullptr;
        };

        // Instance/Function/Property checks need a live object, so results depend on
        // where you run this: main menu resolves only the /Script/ classes; a loaded
        // world (standing in your base) resolves everything.
        constexpr std::array k_checks = std::to_array<Check>({
            {Kind::ScriptClass, ID::Path_PalPlayerCharacter},
            {Kind::ScriptClass, ID::Path_PalBaseCampModel},

            {Kind::Instance, ID::PalPlayerCharacter},
            {Kind::Instance, ID::PalBaseCampManager},
            {Kind::Instance, ID::PalInvaderManager},
            {Kind::Instance, ID::PalWorkBase},
            {Kind::Instance, ID::BP_MonsterAIController_BaseCamp},

            {Kind::Function, ID::PalPlayerCharacter, ID::Fn_K2GetActorLocation},
            {Kind::Function, ID::PalPlayerCharacter, ID::Fn_GetCharacterParameterComponent},
            {Kind::Function, ID::SceneComponent, ID::Fn_K2GetComponentLocation},
            {Kind::Function, ID::PalBaseCampManager, ID::Fn_GetNearestBaseCamp},
            {Kind::Function, ID::PalInvaderManager, ID::Fn_RequestIncidentInvaderEnemy},
            {Kind::Function, ID::PalInvaderManager, ID::Fn_StartInvaderMarchAll},
            {Kind::Function, ID::PalInvaderManager, ID::Fn_RemoveInvaderIncident},
            {Kind::Function, ID::PalCharacterParameterComponent, ID::Fn_GetWork},
            {Kind::Function, ID::PalCharacterParameterComponent, ID::Fn_IsAssignedFixed},
            {Kind::Function, ID::BP_MonsterAIController_BaseCamp, ID::Fn_SetBaseCampActionWithFixAssign},
            {Kind::Function, ID::BP_MonsterAIController_BaseCamp, ID::Fn_SetDefaultPositionAction},

            {Kind::FunctionParam, ID::PalBaseCampManager, ID::Fn_GetNearestBaseCamp, ID::Param_Location},
            {Kind::FunctionParam, ID::PalInvaderManager, ID::Fn_RequestIncidentInvaderEnemy, ID::Param_OccuredBaseCamp},
            {Kind::FunctionParam, ID::PalInvaderManager, ID::Fn_RemoveInvaderIncident, ID::Param_Incident},
            {Kind::FunctionParam, ID::BP_MonsterAIController_BaseCamp, ID::Fn_SetBaseCampActionWithFixAssign, ID::Param_DistanceFixAssign},

            {Kind::Property, ID::PalWorkBase, ID::Prop_Transform},
        });

        enum class Result
        {
            Pass,
            Fail, // container resolved, but the named member/class does not exist -> a rename broke us
            Skip, // no live instance to check against (normal in the main menu / away from base)
        };

        // Human-readable identity of a check, e.g. "PalInvaderManager.RemoveInvaderIncident(Incident)".
        auto describe(const Check& c) -> StringType
        {
            StringType s = c.target;
            if (c.member) { s += STR("."); s += c.member; }
            if (c.param) { s += STR("("); s += c.param; s += STR(")"); }
            return s;
        }

#pragma warning(push)
#pragma warning(disable : 4996) // ForEachProperty deprecated; simplest iterator.

        auto run_check(const Check& c) -> Result
        {
            if (c.kind == Kind::ScriptClass)
            {
                return UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, c.target) ? Result::Pass : Result::Fail;
            }

            UObject* instance = UObjectGlobals::FindFirstOf(c.target);
            if (!instance) { return Result::Skip; }
            if (c.kind == Kind::Instance) { return Result::Pass; }

            if (c.kind == Kind::Property)
            {
                return instance->GetPropertyByNameInChain(c.member) ? Result::Pass : Result::Fail;
            }

            UFunction* fn = instance->GetFunctionByNameInChain(c.member);
            if (!fn) { return Result::Fail; }
            if (c.kind == Kind::Function) { return Result::Pass; }

            // Kind::FunctionParam: the function exists -- does the named parameter still?
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->GetName() == c.param) { return Result::Pass; }
            }
            return Result::Fail;
        }

#pragma warning(pop)
    }

    auto SanityCheckTool::command() const -> StringViewType { return STR("sanity"); }
    auto SanityCheckTool::help() const -> StringViewType
    {
        return STR("verify all known game identifiers against this game version (run in-world, at your base)");
    }

    auto SanityCheckTool::execute(const std::vector<StringType>&, Out& out) -> void
    {
        int pass = 0, fail = 0, skip = 0;
        for (const Check& c : k_checks)
        {
            switch (run_check(c))
            {
            case Result::Pass:
                ++pass;
                break;
            case Result::Fail:
                ++fail;
                say(out, STR("sanity FAIL: {}"), describe(c));
                break;
            case Result::Skip:
                ++skip;
                say(out, STR("sanity skip: {} (no live object -- load a world / stand in your base)"), describe(c));
                break;
            }
        }

        say(out, STR("sanity: {} PASS, {} FAIL, {} skipped (of {})"),
            pass, fail, skip, static_cast<int>(k_checks.size()));
        if (fail > 0)
        {
            say(out, STR("a FAIL means a game patch renamed that identifier -> fix it in core/KnownIdentifiers.hpp"));
        }
    }
}
