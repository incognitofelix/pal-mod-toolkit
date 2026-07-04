// Shared.hpp first: see the note in PlayerLocationTool.cpp.
#include "Shared.hpp"
#include "core/Console.hpp"
#include "core/KnownIdentifiers.hpp"
#include "core/Player.hpp"
#include "tools/ListWorksTool.hpp"

#include <Unreal/UFunction.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UnrealFlags.hpp>

#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    namespace
    {
#pragma warning(push)
#pragma warning(disable : 4996) // ForEachProperty deprecated; still the simplest iterator.

        auto read_object_prop(UObject* obj, const TCHAR* prop_name) -> UObject*
        {
            auto* p = obj->GetPropertyByNameInChain(prop_name);
            if (!p) { return nullptr; }
            return *reinterpret_cast<UObject* const*>(
                reinterpret_cast<uint8_t*>(obj) + p->GetOffset_Internal());
        }

        // Calls a parameterless UFunction returning an FVector (K2_GetComponentLocation).
        auto call_vector_getter(UObject* self, const TCHAR* func_name) -> FVector
        {
            FVector out(0.0, 0.0, 0.0);
            auto* fn = self->GetFunctionByNameInChain(func_name);
            if (!fn) { return out; }
            std::vector<uint8_t> frame(static_cast<size_t>(fn->GetStructureSize()), 0);
            self->ProcessEvent(fn, frame.data());
            for (FProperty* p : fn->ForEachProperty())
            {
                if (p->HasAnyPropertyFlags(CPF_ReturnParm))
                {
                    std::memcpy(&out, frame.data() + p->GetOffset_Internal(), sizeof(FVector));
                    break;
                }
            }
            return out;
        }

#pragma warning(pop)
    }

    auto ListWorksTool::command() const -> StringViewType { return STR("works"); }
    auto ListWorksTool::help() const -> StringViewType
    {
        return STR("list the base's work objects near you with world positions (-> UE4SS.log)");
    }

    auto ListWorksTool::execute(const std::vector<StringType>&, Out& out) -> void
    {
        auto* player = current_player(out);
        if (!player) { say(out, STR("no player found")); return; }
        const FVector pl = player->K2_GetActorLocation();

        std::vector<UObject*> works;
        UObjectGlobals::FindAllOf(Identifiers::PalWorkBase, works);

        constexpr double near_cm = 30000.0; // 300 m -> the base around the player
        int with_transform = 0, with_loc = 0, shown = 0;

        for (auto* w : works)
        {
            auto* t = read_object_prop(w, Identifiers::Prop_Transform);
            if (t) { ++with_transform; }
            const FVector loc = t ? call_vector_getter(t, Identifiers::Fn_K2GetComponentLocation)
                                  : FVector(0.0, 0.0, 0.0);
            const bool has_loc = loc.X() != 0.0 || loc.Y() != 0.0 || loc.Z() != 0.0;
            if (has_loc) { ++with_loc; }

            const double dx = loc.X() - pl.X();
            const double dy = loc.Y() - pl.Y();
            const double dz = loc.Z() - pl.Z();
            const double dist = std::sqrt(dx * dx + dy * dy + dz * dz);

            if (has_loc && dist < near_cm && shown < 40)
            {
                Output::send<LogLevel::Warning>(STR("[Works] {}  dist={}m\n"),
                                                w->GetName(), dist / 100.0);
                ++shown;
            }
        }

        say(out, STR("works total={} withTransform={} withLoc={} shownNear={} (details in UE4SS.log)"),
            works.size(), with_transform, with_loc, shown);
    }
}
