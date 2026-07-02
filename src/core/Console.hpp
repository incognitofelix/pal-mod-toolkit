#pragma once

// Console helpers: format output to the UE4SS log (always) and the in-game console
// (when present), and split a typed command line into tokens.

#include "Shared.hpp"
#include "core/Tool.hpp"

#include <Unreal/FOutputDevice.hpp>

#include <fmt/format.h>
#include <fmt/xchar.h>

#include <vector>

namespace PMT
{
    // Print a formatted line to the in-game console (if any) AND the UE4SS log.
    template <typename... Args>
    auto say(Out& out, fmt::wformat_string<Args...> fmt_str, Args&&... args) -> void
    {
        RC::StringType line = fmt::format(fmt_str, std::forward<Args>(args)...);
        if (out.console) { out.console->Log(line.c_str()); }
        RC::Output::send<RC::LogLevel::Warning>(STR("[pmt] {}\n"), line);
    }

    // Split a console/chat command line into whitespace-separated tokens.
    inline auto tokenize(const TCHAR* cmd) -> std::vector<RC::StringType>
    {
        std::vector<RC::StringType> tokens;
        RC::StringType cur;
        for (const TCHAR* p = cmd; p && *p; ++p)
        {
            if (*p == ' ' || *p == '\t')
            {
                if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
            }
            else
            {
                cur += *p;
            }
        }
        if (!cur.empty()) { tokens.push_back(cur); }
        return tokens;
    }
}
