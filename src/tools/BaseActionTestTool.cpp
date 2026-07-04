// Shared.hpp first: see the note in PlayerLocationTool.cpp.
#include "Shared.hpp"
#include "core/Console.hpp"
#include "core/KnownIdentifiers.hpp"
#include "tools/BaseActionTestTool.hpp"

#include <Unreal/UFunction.hpp>

#include <vector>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    auto BaseActionTestTool::command() const -> StringViewType { return STR("defaultpos"); }
    auto BaseActionTestTool::help() const -> StringViewType
    {
        return STR("send every worker Pal to its default position (diagnostic)");
    }

    auto BaseActionTestTool::execute(const std::vector<StringType>&, Out& out) -> void
    {
        // Every worker Pal in a base is driven by a BP_MonsterAIController_BaseCamp_C.
        std::vector<UObject*> controllers;
        UObjectGlobals::FindAllOf(Identifiers::BP_MonsterAIController_BaseCamp, controllers);
        if (controllers.empty()) { say(out, STR("no base-camp controllers found")); return; }

        // SetDefaultPositionAction() is parameterless -> ProcessEvent with no parm buffer.
        // Visible effect: the Pals stop working and walk to their default position.
        int called = 0;
        for (auto* ctrl : controllers)
        {
            auto* fn = ctrl->GetFunctionByNameInChain(Identifiers::Fn_SetDefaultPositionAction);
            if (!fn) { continue; }
            ctrl->ProcessEvent(fn, nullptr);
            ++called;
        }
        say(out, STR("SetDefaultPositionAction() called on {} Pal(s)"), called);
    }
}
