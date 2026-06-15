#pragma once

// Common UE4SS / Unreal headers used across the toolkit.
// NOTE: intentionally no `using namespace` here — headers should not pollute the
// global namespace of everything that includes them. The .cpp files pull in the
// `using` directives locally.

#include <Mod/CppUserModBase.hpp>
#include <DynamicOutput/DynamicOutput.hpp>
#include <Unreal/UObjectGlobals.hpp>
#include <Unreal/AActor.hpp>
#include <Unreal/UnrealCoreStructs.hpp>
