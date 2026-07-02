// Shared.hpp first: see the note in PlayerLocationTool.cpp.
#include "Shared.hpp"
#include "core/Console.hpp"
#include "tools/WorkCaptureTool.hpp"

#include <Unreal/Hooks.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UnrealFlags.hpp>

#include <array>
#include <unordered_set>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    namespace
    {
        // Capture state. The global ProcessEvent callback can only be registered (not
        // removed), so it is gated by s_active and registered exactly once.
        bool s_active = false;
        bool s_registered = false;
        std::unordered_set<StringType> s_seen;

        const std::array<StringType, 4> k_capture_keywords = {
            STR("Work"), STR("Assign"), STR("Reserve"), STR("Liftup"),
        };

        auto has_keyword(const StringType& text) -> bool
        {
            for (const auto& kw : k_capture_keywords)
            {
                if (text.find(kw) != StringType::npos) { return true; }
            }
            return false;
        }

        // MSVC C4996: deprecated ForEachProperty still returns a usable TFieldRange.
#pragma warning(push)
#pragma warning(disable : 4996)

        // Builds "name:type" for each input parameter, and for object-like params appends
        // the pointed object's name -> identifies WHICH Pal / building / work a call means.
        //
        // SAFETY: only real input params (CPF_Parm, not Out/Return) are dereferenced -- a
        // Pre-callback runs before the function fills its Out/Return memory, so reading those
        // means reading uninitialised garbage (that caused an access violation). Out/Return
        // are listed as "(out)" only. Object pointers also get a coarse sanity check.
        auto describe_params(UFunction* function, void* parms) -> StringType
        {
            StringType out{};
            if (!parms) { return out; }
            for (FProperty* param : function->ForEachProperty())
            {
                if (!param->HasAnyPropertyFlags(CPF_Parm)) { continue; }

                const StringType ptype = param->GetClass().GetName();
                if (!out.empty()) { out += STR(", "); }
                out += param->GetName();
                out += STR(":");
                out += ptype;

                if (param->HasAnyPropertyFlags(static_cast<EPropertyFlags>(CPF_OutParm | CPF_ReturnParm)))
                {
                    out += STR("(out)");
                    continue;
                }

                if (ptype == STR("ObjectProperty") || ptype == STR("InterfaceProperty"))
                {
                    auto* base = static_cast<uint8_t*>(parms) + param->GetOffset_Internal();
                    auto* obj = *reinterpret_cast<UObject* const*>(base);
                    out += STR("=");
                    if (reinterpret_cast<uintptr_t>(obj) > 0x10000)
                    {
                        out += obj->GetName();
                    }
                    else
                    {
                        out += STR("(null)");
                    }
                }
            }
            return out;
        }

#pragma warning(pop)

        // Runs on every ProcessEvent call. Keep the early-outs cheap.
        auto on_process_event(UObject* /*context*/, UFunction* function, void* parms) -> void
        {
            if (!s_active || !function) { return; }

            const StringType fname = function->GetName();
            if (!has_keyword(fname)) { return; }

            // GetFullName() = "Function /Script/Pal.<Class>:<Func>" -> defining class too.
            const StringType full = function->GetFullName();
            const StringType params = describe_params(function, parms);
            const StringType line = full + STR("  (") + params + STR(")");

            // Dedupe per (function + target params): the same call with a new Pal/building
            // logs again, so your drop separates from the base's background reassignments.
            if (s_seen.find(line) != s_seen.end()) { return; }
            s_seen.insert(line);

            Output::send<LogLevel::Warning>(STR("[Capture] {}\n"), line);
        }
    }

    auto WorkCaptureTool::command() const -> StringViewType { return STR("capture"); }
    auto WorkCaptureTool::help() const -> StringViewType
    {
        return STR("toggle live capture of worker/assignment function calls (-> UE4SS.log)");
    }

    auto WorkCaptureTool::execute(const std::vector<StringType>&, Out& out) -> void
    {
        if (!s_registered)
        {
            Hook::RegisterProcessEventPreCallback(
                [](UObject* c, UFunction* f, void* p) { on_process_event(c, f, p); });
            s_registered = true;
        }

        s_active = !s_active;
        if (s_active)
        {
            s_seen.clear();
            say(out, STR("capture ARMED -- now perform the action (e.g. drop a Pal)"));
        }
        else
        {
            say(out, STR("capture STOPPED (results in UE4SS.log)"));
        }
    }
}
