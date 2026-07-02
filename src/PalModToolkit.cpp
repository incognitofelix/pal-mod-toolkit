#include "Shared.hpp"
#include "core/Tool.hpp"
#include "core/Console.hpp"
#include "tools/PlayerLocationTool.hpp"
#include "tools/NearbyActorsTool.hpp"
#include "tools/BaseReconTool.hpp"
#include "tools/WorkCaptureTool.hpp"
#include "tools/BaseActionTestTool.hpp"
#include "tools/AssignTestTool.hpp"
#include "tools/ListWorksTool.hpp"
#include "tools/RaidTool.hpp"

#include <Unreal/Hooks.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UnrealFlags.hpp>
#include <Unreal/Core/Containers/FString.hpp>
#include <Unreal/FText.hpp>

#include <UE4SSProgram.hpp>
#include <imgui.h>

#include <memory>
#include <mutex>
#include <vector>

using namespace RC;
using namespace RC::Unreal;

namespace PMT
{
    // The toolkit core. It owns a list of tools and dispatches console commands of the
    // form "pmt <command> [args]" to them. Console input comes from the native UE console
    // (Tilde / F10, enabled by ConsoleEnablerMod); we hook UGameViewportClient::ProcessConsoleExec.
    class PalModToolkit final : public CppUserModBase
    {
    public:
        PalModToolkit()
        {
            ModName        = STR("PalModToolkit");
            ModVersion     = STR("0.2.0");
            ModDescription = STR("A console-driven toolkit of in-game tools for Palworld modders.");
            ModAuthors     = STR("felix");

            Output::send<LogLevel::Verbose>(STR("[PalModToolkit] v0.2.0 loading...\n"));

            // ---- register tools here ----
            register_tool(std::make_unique<PlayerLocationTool>());
            register_tool(std::make_unique<NearbyActorsTool>());
            register_tool(std::make_unique<BaseReconTool>());
            register_tool(std::make_unique<WorkCaptureTool>());
            register_tool(std::make_unique<ListWorksTool>());
            register_tool(std::make_unique<BaseActionTestTool>());
            register_tool(std::make_unique<AssignTestTool>());
            register_tool(std::make_unique<RaidTool>());
            // -----------------------------

            Output::send<LogLevel::Verbose>(STR("[PalModToolkit] {} command(s) ready. Type 'pmt help' in the console.\n"),
                                            m_tools.size());
        }

        // Register the console hook once the Unreal module is up.
        auto on_unreal_init() -> void override
        {
            // Input path 1: the native UE console (if it opens).
            Hook::RegisterProcessConsoleExecCallback(
                [this](UObject*, const TCHAR* cmd, FOutputDevice& ar, UObject* executor) -> bool
                {
                    Out out{ &ar, executor };
                    return handle_console(cmd, out);
                });

            // Input path 2: in-game chat -- intercept "pmt ..." typed in the chat box.
            // Works on a dedicated server too (the chat RPC reaches the server process).
            Hook::RegisterProcessEventPreCallback(
                [this](UObject* c, UFunction* f, void* p)
                {
                    try_handle_chat(c, f, p);
                });

            Output::send<LogLevel::Verbose>(STR("[PalModToolkit] input hooks registered (console + chat).\n"));
        }

        // Input path 3: our own command input in the UE4SS debug window (works in SP/client).
        auto on_ui_init() -> void override
        {
            UE4SS_ENABLE_IMGUI();
            register_tab(STR("PalModToolkit"), &render_tab_thunk);
        }

        // Commands typed in the tab are captured on the GUI thread and run here (game thread).
        auto on_update() -> void override
        {
            std::vector<StringType> cmds;
            {
                std::lock_guard<std::mutex> lk(m_mutex);
                cmds.swap(m_pending);
            }
            for (auto& c : cmds)
            {
                Out out{ nullptr, nullptr };
                handle_console(c.c_str(), out);
            }
        }

    private:
        auto register_tool(std::unique_ptr<Tool> tool) -> void
        {
            Output::send<LogLevel::Verbose>(STR("[PalModToolkit]   - pmt {}\n"), tool->command());
            m_tools.push_back(std::move(tool));
        }

        // Returns true if the command was ours (handled), so the game ignores it.
        auto handle_console(const TCHAR* cmd, Out& out) -> bool
        {
            const auto tokens = tokenize(cmd);
            if (tokens.empty() || tokens[0] != STR("pmt")) { return false; }

            if (tokens.size() < 2 || tokens[1] == STR("help"))
            {
                print_help(out);
                return true;
            }

            const StringType& sub = tokens[1];
            const std::vector<StringType> args(tokens.begin() + 2, tokens.end());

            for (auto& tool : m_tools)
            {
                if (StringType(tool->command()) == sub)
                {
                    tool->execute(args, out);
                    return true;
                }
            }

            say(out, STR("unknown command '{}'. Try 'pmt help'."), sub);
            return true;
        }

        auto print_help(Out& out) -> void
        {
            say(out, STR("PalModToolkit commands:"));
            for (auto& tool : m_tools)
            {
                say(out, STR("  pmt {} - {}"), StringType(tool->command()), StringType(tool->help()));
            }
        }

        // If `fn` is a chat-send carrying a "pmt ..." message, run it (log output) and
        // blank the message so it does not post to chat.
#pragma warning(push)
#pragma warning(disable : 4996) // ForEachProperty deprecated; simplest iterator.
        auto try_handle_chat(UObject* context, UFunction* fn, void* parms) -> void
        {
            if (!fn || !parms) { return; }
            const StringType name = fn->GetName();
            // Catch chat sends and chat-box text commits (the typed line).
            const bool candidate = name.find(STR("Chat")) != StringType::npos ||
                                   name.find(STR("Committed")) != StringType::npos;
            if (!candidate) { return; }

            for (FProperty* p : fn->ForEachProperty())
            {
                if (!p->HasAnyPropertyFlags(CPF_Parm)) { continue; }
                if (p->HasAnyPropertyFlags(static_cast<EPropertyFlags>(CPF_OutParm | CPF_ReturnParm))) { continue; }

                const StringType ptype = p->GetClass().GetName();
                auto* at = static_cast<uint8_t*>(parms) + p->GetOffset_Internal();

                StringType msg;
                if (ptype == STR("StrProperty"))
                {
                    auto* fs = reinterpret_cast<FString*>(at);
                    if (fs->Len() > 0) { msg = StringType(**fs); }
                }
                else if (ptype == STR("TextProperty"))
                {
                    msg = reinterpret_cast<FText*>(at)->ToString();
                }
                else
                {
                    continue;
                }

                if (msg.rfind(STR("pmt"), 0) == 0)
                {
                    Out out{ nullptr, context }; // chat path: log-only, issuer = context
                    handle_console(msg.c_str(), out);
                    if (ptype == STR("StrProperty")) { reinterpret_cast<FString*>(at)->Clear(); }
                }
                return; // only inspect the first string-ish parameter
            }
        }
#pragma warning(pop)

        static auto render_tab_thunk(CppUserModBase* mod) -> void
        {
            static_cast<PalModToolkit*>(mod)->render_tab_content();
        }

        auto render_tab_content() -> void
        {
            ImGui::TextUnformatted("PalModToolkit - type a command, then Enter (e.g. 'pmt help'):");
            bool run = ImGui::InputText("##pmtcmd", m_cmd_buf, sizeof(m_cmd_buf),
                                        ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::SameLine();
            if (ImGui::Button("Run")) { run = true; }
            if (run && m_cmd_buf[0] != '\0')
            {
                StringType cmd;
                for (const char* p = m_cmd_buf; *p; ++p)
                {
                    cmd += static_cast<wchar_t>(static_cast<unsigned char>(*p));
                }
                {
                    std::lock_guard<std::mutex> lk(m_mutex);
                    m_pending.push_back(cmd);
                }
                m_cmd_buf[0] = '\0';
            }
            ImGui::TextUnformatted("Output appears in the UE4SS log (main output area).");
        }

        std::vector<std::unique_ptr<Tool>> m_tools;
        char m_cmd_buf[256] = {};
        std::mutex m_mutex;
        std::vector<StringType> m_pending;
    };
}

// ---- UE4SS C++ mod entry points ----
#define PMT_API __declspec(dllexport)
extern "C"
{
    PMT_API RC::CppUserModBase* start_mod()
    {
        return new PMT::PalModToolkit();
    }

    PMT_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
