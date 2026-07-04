#pragma once

// Every game-side identifier this toolkit resolves at runtime, in one place.
//
// These strings are the toolkit's ONLY coupling to Palworld's internals: we find
// everything through UE4SS reflection (by name), never through memory offsets. That
// makes the mod resilient to game patches -- except when a patch renames one of these.
// Central constants mean (a) a rename is a one-line fix, and (b) SanityCheckTool can
// verify every entry against the running game ("pmt sanity") instead of tools failing
// silently one by one.
//
// Stability rule of thumb:
//   /Script/...  = native C++ classes -- rarely renamed by patches.
//   ..._C        = Blueprint-generated classes -- renamed/moved more often.
//   function/param names = reflection metadata -- can change with any game update.
//
// `inline constexpr`: constexpr alone would give each .cpp its own copy (fine for
// pointers, but ODR-murky for anything bigger); inline guarantees one entity across
// all translation units that include this header.

#include <String/StringType.hpp> // STR()

namespace PMT::Identifiers
{
    // ---- Class names, resolved against live instances (FindFirstOf / FindAllOf) ----
    inline constexpr const auto* PalPlayerCharacter = STR("PalPlayerCharacter");
    inline constexpr const auto* Actor = STR("Actor");
    inline constexpr const auto* SceneComponent = STR("SceneComponent");
    inline constexpr const auto* PalBaseCampManager = STR("PalBaseCampManager");
    inline constexpr const auto* PalInvaderManager = STR("PalInvaderManager");
    inline constexpr const auto* PalWorkBase = STR("PalWorkBase");
    inline constexpr const auto* PalCharacterParameterComponent = STR("PalCharacterParameterComponent");
    inline constexpr const auto* BP_MonsterAIController_BaseCamp = STR("BP_MonsterAIController_BaseCamp_C");

    // ---- Full object paths (StaticFindObject) ----
    inline constexpr const auto* Path_PalPlayerCharacter = STR("/Script/Pal.PalPlayerCharacter");
    inline constexpr const auto* Path_PalBaseCampModel = STR("/Script/Pal.PalBaseCampModel");
    // Prefix used by "pmt recon <BareName>" to expand bare names.
    inline constexpr const auto* Path_ScriptPalPrefix = STR("/Script/Pal.");

    // ---- UFunction names (GetFunctionByNameInChain on an instance) ----
    inline constexpr const auto* Fn_K2GetActorLocation = STR("K2_GetActorLocation");
    inline constexpr const auto* Fn_K2GetPawn = STR("K2_GetPawn");
    inline constexpr const auto* Fn_GetOwner = STR("GetOwner");
    inline constexpr const auto* Fn_K2GetComponentLocation = STR("K2_GetComponentLocation");
    inline constexpr const auto* Fn_GetNearestBaseCamp = STR("GetNearestBaseCamp");
    inline constexpr const auto* Fn_RequestIncidentInvaderEnemy = STR("RequestIncidentInvaderEnemy_BP");
    inline constexpr const auto* Fn_StartInvaderMarchAll = STR("StartInvaderMarchAll");
    inline constexpr const auto* Fn_RemoveInvaderIncident = STR("RemoveInvaderIncident");
    inline constexpr const auto* Fn_GetCharacterParameterComponent = STR("GetCharacterParameterComponent");
    inline constexpr const auto* Fn_GetWork = STR("GetWork");
    inline constexpr const auto* Fn_IsAssignedFixed = STR("IsAssignedFixed");
    inline constexpr const auto* Fn_SetBaseCampActionWithFixAssign = STR("SetBaseCampActionWithFixAssign");
    inline constexpr const auto* Fn_SetDefaultPositionAction = STR("SetDefaultPositionAction");

    // ---- UFunction PARAMETER names ----
    // Tools write arguments into ProcessEvent frames by looking these up per name; a
    // renamed param doesn't crash -- the write silently never happens. Extra sneaky,
    // so the sanity check covers them explicitly.
    inline constexpr const auto* Param_Location = STR("Location");                 // GetNearestBaseCamp
    inline constexpr const auto* Param_OccuredBaseCamp = STR("OccuredBaseCamp");   // RequestIncidentInvaderEnemy_BP
    inline constexpr const auto* Param_Incident = STR("Incident");                 // RemoveInvaderIncident
    inline constexpr const auto* Param_DistanceFixAssign = STR("DistanceFixAssignTargetting"); // SetBaseCampActionWithFixAssign

    // ---- FProperty names (GetPropertyByNameInChain) ----
    inline constexpr const auto* Prop_Transform = STR("Transform"); // PalWorkBase
}
