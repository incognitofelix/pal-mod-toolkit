#pragma once

#include <Input/KeyDef.hpp>
#include <String/StringType.hpp>

namespace PMT
{
    // Abstract base class for every toolkit tool.
    //
    // A "tool" is one self-contained feature, bound to a hotkey. To add a new tool:
    //   1. Create a class that derives from Tool and implements the three methods below.
    //   2. Register it once in PalModToolkit's constructor.
    // The toolkit core takes care of wiring the hotkey to on_activate().
    class Tool
    {
    public:
        virtual ~Tool() = default;

        // Human-readable name, shown in the load log.
        virtual auto name() const -> RC::StringViewType = 0;

        // The key that triggers this tool.
        virtual auto hotkey() const -> RC::Input::Key = 0;

        // Runs each time the hotkey is pressed.
        virtual auto on_activate() -> void = 0;
    };
}
