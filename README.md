# PalModToolkit

A small **toolkit of in-game tools for Palworld modders**, built as a single
[UE4SS](https://github.com/UE4SS-RE/RE-UE4SS) C++ mod. It is **console-driven**: type
`pmt <command>` and get information printed to the UE4SS log — handy while
reverse-engineering the game.

> Status: early. Built for Palworld + the Palworld-specific UE4SS (`experimental-palworld`).

## Using it

The toolkit adds a **`PalModToolkit` tab** to the UE4SS debug window (enabled via
`GuiConsoleEnabled` in `UE4SS-settings.ini`). Open that window, select the tab, type a
command into the input box and press Enter. Output appears in the UE4SS log.

> It also listens on the native UE console (`UGameViewportClient::ProcessConsoleExec`)
> and in-game chat where those are available, but the debug-window tab is the reliable
> path (it works in single-player and is independent of the game UI).

Type `pmt help` to list everything. Commands:

| Command | What it does |
|---------|--------------|
| `pmt help` | List all commands. |
| `pmt loc` | Print the issuing player's world position (X/Y/Z, cm). |
| `pmt actors [meters]` | List every Actor within a radius (default 50 m) — distance + class/path. Details → UE4SS.log. |
| `pmt recon <Name\|/Script/Pkg.Class> ...` | Reflection-dump the given classes (properties + functions, walking the native super chain) to the log, flagging keyword `HIT`s. A bare name resolves to `/Script/Pal.<Name>`. |
| `pmt capture` | Toggle. While armed, logs every worker/assignment function call (with parameter values) — e.g. while assigning a Pal — to reveal the native call chain. |
| `pmt works` | List the base's work objects near you with world positions. |
| `pmt defaultpos` | Send every worker Pal to its default position (diagnostic). |
| `pmt assign` | Teleport the first base Pal to you and fix-assign it (the native "V-drop" backend). |

Position/distance commands resolve **the player who issued the command** (console
executor / chat sender), so they work correctly in multiplayer and on a server.

## Architecture

One DLL, internally modular. The core (`src/PalModToolkit.cpp`) owns a list of
**tools**; each tool is a small class deriving from `PMT::Tool` (`src/core/Tool.hpp`)
with `command()`, `help()` and `execute(args, out)`. The core parses a typed line and
dispatches to the matching tool. Input typed in the debug-window tab is captured on the
GUI thread and executed on the game thread (`on_update`) for safety.

**Add a new tool** in three steps:

1. Create `src/tools/MyTool.hpp` + `.cpp` deriving from `PMT::Tool` and implement
   `command()`, `help()`, `execute()`.
2. `#include "tools/MyTool.hpp"` in `src/PalModToolkit.cpp`.
3. Add `register_tool(std::make_unique<MyTool>());` in the constructor.

No build-file changes needed — `src/**.cpp` is globbed automatically.

## Building

**Prerequisites**

- Visual Studio 2022 with the *Desktop development with C++* workload (MSVC).
- [xmake](https://xmake.io).
- The **RE-UE4SS source tree** (for headers). This requires an Epic-Games-linked
  GitHub account to fetch the gated `UEPseudo` submodule — see the
  [UE4SS build guide](https://docs.ue4ss.com/dev/guides/installation.html).
- An installed, running **UE4SS.dll** for your game (the import library is generated
  from it).

**Steps**

```powershell
# 1) Generate the import library from your installed UE4SS.dll (run once / after UE4SS updates)
.\scripts\generate_import_lib.ps1 -Ue4ssDll "C:\...\Palworld\Pal\Binaries\Win64\ue4ss\UE4SS.dll"

# 2) Configure (point xmake at your RE-UE4SS source + the import lib folder)
#    From an "x64 Native Tools Command Prompt for VS 2022":
xmake f -m release --vs=2022 ^
  --ue4ss_src="C:/path/to/RE-UE4SS" ^
  --ue4ss_implib="C:/path/to/PalModToolkit/external/ue4ss_implib" -y

# 3) Build
xmake build PalModToolkit
```

The output `main.dll` lands in `build/windows/x64/release/`.

## Installing

Copy the built DLL into your UE4SS `Mods` folder:

```
<Game>\...\Win64\ue4ss\Mods\PalModToolkit\dlls\main.dll
<Game>\...\Win64\ue4ss\Mods\PalModToolkit\enabled.txt   (empty file)
```

Launch the game; the toolkit announces itself and its tools in the UE4SS console.

## Notes on distribution

This mod **links against the UE4SS.dll the game loads** rather than recompiling
UE4SS. That guarantees an exact ABI match and avoids building UE4SS's GUI
dependencies — but it means each builder generates their own import library from
their own UE4SS.dll (step 1 above). The library is intentionally **not** committed.

## License

MIT — see [LICENSE](LICENSE).
