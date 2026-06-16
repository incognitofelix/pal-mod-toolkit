# PalModToolkit

A small **toolkit of in-game tools for Palworld modders**, built as a single
[UE4SS](https://github.com/UE4SS-RE/RE-UE4SS) C++ mod. Press a hotkey, get
information printed to the UE4SS console — handy while reverse-engineering the game.

> Status: early. Built for Palworld + the Palworld-specific UE4SS (`experimental-palworld`).

## Tools

| Hotkey | Tool | What it does |
|--------|------|--------------|
| `Shift+F1` | Player Location | Prints the local player's world position (X/Y/Z, cm). |
| `Shift+F2` | Nearby Actors | Lists every Actor within 50 m of the player (distance + class/path). Great for finding spawners, dungeons and map objects at a spot. |
| `Shift+F3` | Base Recon | Reflection-dumps base-camp / worker / work classes (properties + functions, walking the native super chain) to the UE4SS log, flagging keyword `HIT`s. For mapping the worker-assignment API. |
| `Shift+F4` | Work Capture | Toggle. While armed, logs every worker/assignment-related function call (with parameter values) as it happens — e.g. while assigning a Pal — to reveal the native call chain. |

> `Shift` is used as the modifier because Ctrl (dodge) and Alt (Windows shortcuts) are taken; Shift (sprint) only fires together with movement, so it stays conflict-free with the F-keys.

## Architecture

One DLL, internally modular. The core (`src/PalModToolkit.cpp`) owns a list of
**tools**; each tool is a small self-contained class deriving from `PMT::Tool`
(`src/core/Tool.hpp`). The core wires each tool's hotkey to its `on_activate()`.

**Add a new tool** in three steps:

1. Create `src/tools/MyTool.hpp` + `.cpp` deriving from `PMT::Tool` and implement
   `name()`, `hotkey()`, `on_activate()`.
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
