#pragma once

// Resolves "the player who issued the command" to their PalPlayerCharacter, so that
// position/distance commands reference the correct player on a dedicated server / in
// multiplayer (where FindFirstOf would grab an arbitrary player).

#include "Shared.hpp"
#include "core/KnownIdentifiers.hpp"
#include "core/Tool.hpp"

#include <Unreal/AActor.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UnrealFlags.hpp>

#include <vector>
#include <cstdint>

namespace PMT
{
#pragma warning(push)
#pragma warning(disable : 4996) // ForEachProperty deprecated; simplest iterator.

    // Calls a parameterless UFunction returning a UObject* (K2_GetPawn, GetOwner, ...).
    inline auto pmt_call_object(RC::Unreal::UObject* self, const TCHAR* func_name) -> RC::Unreal::UObject*
    {
        using namespace RC::Unreal;
        if (!self) { return nullptr; }
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

#pragma warning(pop)

    // The character of the command's issuer, or (fallback) the first player character.
    inline auto current_player(Out& out) -> RC::Unreal::AActor*
    {
        using namespace RC::Unreal;
        UObject* c = out.player;
        if (c)
        {
            // Controller -> possessed pawn (the character).
            if (auto* pawn = pmt_call_object(c, Identifiers::Fn_K2GetPawn)) { return static_cast<AActor*>(pawn); }
            // The issuer is itself an actor (the character).
            if (c->GetFunctionByNameInChain(Identifiers::Fn_K2GetActorLocation)) { return static_cast<AActor*>(c); }
            // A component -> its owner -> (pawn or actor).
            if (auto* owner = pmt_call_object(c, Identifiers::Fn_GetOwner))
            {
                if (auto* pawn = pmt_call_object(owner, Identifiers::Fn_K2GetPawn)) { return static_cast<AActor*>(pawn); }
                if (owner->GetFunctionByNameInChain(Identifiers::Fn_K2GetActorLocation)) { return static_cast<AActor*>(owner); }
            }
        }
        return static_cast<AActor*>(UObjectGlobals::FindFirstOf(Identifiers::PalPlayerCharacter));
    }
}
