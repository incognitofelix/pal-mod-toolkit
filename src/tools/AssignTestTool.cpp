// Shared.hpp first: see the note in PlayerLocationTool.cpp.
#include "Shared.hpp"
#include "core/Console.hpp"
#include "core/Player.hpp"
#include "tools/AssignTestTool.hpp"

#include <Unreal/FHitResult.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UnrealFlags.hpp>

#include <vector>
#include <cstdint>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    namespace
    {
#pragma warning(push)
#pragma warning(disable : 4996) // ForEachProperty deprecated; still the simplest iterator.

        // Calls a parameterless UFunction that returns a UObject* and returns it.
        auto call_object_getter(UObject* self, const TCHAR* func_name) -> UObject*
        {
            auto* fn = self->GetFunctionByNameInChain(func_name);
            if (!fn) { return nullptr; }
            std::vector<uint8_t> frame(static_cast<size_t>(fn->GetStructureSize()), 0);
            self->ProcessEvent(fn, frame.data());
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->HasAnyPropertyFlags(CPF_ReturnParm))
                {
                    return *reinterpret_cast<UObject* const*>(frame.data() + p->GetOffset_Internal());
                }
            }
            return nullptr;
        }

        // Calls a parameterless UFunction returning a bool and returns it.
        auto call_bool_getter(UObject* self, const TCHAR* func_name) -> bool
        {
            auto* fn = self->GetFunctionByNameInChain(func_name);
            if (!fn) { return false; }
            std::vector<uint8_t> frame(static_cast<size_t>(fn->GetStructureSize()), 0);
            self->ProcessEvent(fn, frame.data());
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->HasAnyPropertyFlags(CPF_ReturnParm))
                {
                    return *reinterpret_cast<bool*>(frame.data() + p->GetOffset_Internal());
                }
            }
            return false;
        }

        // BP_MonsterAIController_BaseCamp_C::SetBaseCampActionWithFixAssign(distance)
        auto call_fix_assign(UObject* controller, float distance) -> void
        {
            auto* fn = controller->GetFunctionByNameInChain(STR("SetBaseCampActionWithFixAssign"));
            if (!fn) { return; }
            std::vector<uint8_t> frame(static_cast<size_t>(fn->GetStructureSize()), 0);
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->GetName() == STR("DistanceFixAssignTargetting"))
                {
                    *reinterpret_cast<float*>(frame.data() + p->GetOffset_Internal()) = distance;
                }
            }
            controller->ProcessEvent(fn, frame.data());
        }

#pragma warning(pop)
    }

    auto AssignTestTool::command() const -> StringViewType { return STR("assign"); }
    auto AssignTestTool::help() const -> StringViewType
    {
        return STR("teleport the first base Pal to you and fix-assign it (V-drop backend)");
    }

    auto AssignTestTool::execute(const std::vector<StringType>&, Out& out) -> void
    {
        std::vector<UObject*> controllers;
        UObjectGlobals::FindAllOf(STR("BP_MonsterAIController_BaseCamp_C"), controllers);
        if (controllers.empty()) { say(out, STR("no base-camp controllers")); return; }
        auto* ctrl = controllers[0];

        auto* pawn = call_object_getter(ctrl, STR("K2_GetPawn"));
        if (!pawn) { say(out, STR("no pawn")); return; }
        auto* param = call_object_getter(pawn, STR("GetCharacterParameterComponent"));
        if (!param) { say(out, STR("no param component")); return; }

        // STATE BEFORE this run (= did the previous run's assignment stick?).
        auto* work = call_object_getter(param, STR("GetWork"));
        const bool fixed = call_bool_getter(param, STR("IsAssignedFixed"));
        say(out, STR("{} | currentWork={} fixed={}"),
            pawn->GetName(), work ? work->GetName() : StringType(STR("(none)")), fixed);

        // THE BACKEND = native V-drop logic: put the Pal at the building, fix-assign.
        auto* player = current_player(out);
        if (!player) { return; }
        const FVector target = player->K2_GetActorLocation();

        FHitResult hit{};
        static_cast<AActor*>(pawn)->K2_SetActorLocation(target, false, hit, true);
        call_fix_assign(ctrl, 1000.0f); // bind to the work within 10 m of the drop

        say(out, STR("teleported + fix-assigned. Run 'pmt assign' again here to verify currentWork."));
    }
}
