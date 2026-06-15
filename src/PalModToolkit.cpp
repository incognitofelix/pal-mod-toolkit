#include "Shared.hpp"
#include "core/Tool.hpp"
#include "tools/PlayerLocationTool.hpp"
#include "tools/NearbyActorsTool.hpp"

#include <memory>
#include <vector>

using namespace RC;

namespace PMT
{
    // The toolkit core. It owns a list of tools and wires each tool's hotkey to its
    // on_activate(). Adding a feature is a two-liner: include its header and call
    // register_tool() in the constructor below.
    class PalModToolkit final : public CppUserModBase
    {
    public:
        PalModToolkit()
        {
            ModName        = STR("PalModToolkit");
            ModVersion     = STR("0.1.0");
            ModDescription = STR("A toolkit of in-game tools for Palworld modders.");
            ModAuthors     = STR("felix");

            Output::send<LogLevel::Verbose>(STR("[PalModToolkit] v0.1.0 loading...\n"));

            // ---- register tools here ----
            register_tool(std::make_unique<PlayerLocationTool>());
            register_tool(std::make_unique<NearbyActorsTool>());
            // -----------------------------

            Output::send<LogLevel::Verbose>(STR("[PalModToolkit] {} tool(s) ready.\n"), m_tools.size());
        }

    private:
        // Takes ownership of the tool, binds its hotkey, and stores it.
        auto register_tool(std::unique_ptr<Tool> tool) -> void
        {
            // Raw observer pointer for the lambda. The unique_ptr (and therefore the
            // tool object) outlives every hotkey press because it lives in m_tools,
            // which lives as long as this mod instance.
            Tool* observer = tool.get();
            const auto mods = observer->modifiers();

            // mods[0] == 0 (MOD_KEY_START_OF_ENUM) means the tool wants no modifiers.
            if (mods[0] != Input::ModifierKey::MOD_KEY_START_OF_ENUM)
            {
                register_keydown_event(observer->hotkey(), mods, [observer]() { observer->on_activate(); });
            }
            else
            {
                register_keydown_event(observer->hotkey(), [observer]() { observer->on_activate(); });
            }

            Output::send<LogLevel::Verbose>(STR("[PalModToolkit]   - {}\n"), observer->name());
            m_tools.push_back(std::move(tool));
        }

        std::vector<std::unique_ptr<Tool>> m_tools;
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
