# ViceCity CS2 Cheat

A full-featured Counter-Strike 2 cheat with memesense parity, custom pink-yellow themed GUI, and comprehensive skinchanger/inventory system.

## Features

### RageBot
- **Aimbot**: Multipoint, hitchance, minimum damage, autowall, resolver integration
- **Exploits**: Double Tap, Hide Shots, Fake Lag, Quick Stop, Quick Peek
- **Anti-Aim**: Pitch/Yaw/Desync/LBY/Freestanding/Manual AA
- **Resolver**: Brute force, history-based, ML-ready, animation fix, lag compensation

### LegitBot (VACNet Safe)
- **Triggerbot**: Delay, burst, hitgroup filters, visibility checks
- **Backtrack**: 200ms legit backtrack with visualization
- **Legit AA**: Freestanding, edge yaw, static, LBY
- **Movement**: Bhop, auto strafe, edge jump, fast duck, slow walk, auto peek

### Visuals
- **ESP**: Box (2D/3D/Corner), skeleton, health, armor, ammo, name, weapon, flags, dormant, snaplines, offscreen arrows
- **Chams**: Player/arms/weapons/attachments with materials (flat/glass/plastic/metallic/glow/wireframe/pulse), history, shot chams
- **World**: Nightmode, prop transparency, grenade preview, hitmarker, damage indicator, spread circle
- **Radar**: 2D radar with custom range/icons
- **Effects**: Bullet tracers, impact beams, bullet beams, hit numbers

### Skinchanger / Inventory System (memesense 1:1)
- **Full Inventory**: Weapons, knives, gloves, agents, stickers, cases, keys, music kits, medals, charms, patches
- **Case Opening**: Full animation with rolling, rare item glow, sound effects
- **Sticker Tool**: Apply/scrape/peel, position/rotate/scale/wear per slot
- **Inspect Panel**: 3D model viewer, wear float, pattern seed, sticker closeups, StatTrak
- **Music Kits / MVP Anthems**: Preview, equip, menu music override
- **Agents / Patches**: Model preview, equip regions
- **Gloves**: Wear preview, pattern seed
- **StatTrak Spoof**: Client-side counter increment
- **Rarity Glow**: Border glow per rarity tier
- **Pattern Seed Browser**: Browse seeds 0-1000 with preview
- **Economy Sim**: Fake balance, case/key prices, sell values, market history

### Misc
- Movement: Auto peek, thirdperson, FOV override, viewmodel changer, aspect ratio
- Logs: Hit/damage/purchase/console logs with filtering
- Config System: JSON configs, per-map, cloud sync (Gist), import/export Base64
- Lua Scripting: Menu/Engine/Entity/Render APIs, callbacks, script manager

### GUI (ViceCity Theme)
- Pink-Yellow warmth color scheme
- Custom widgets: Gradient color picker, 4-mode keybind, searchable multi-combo, tooltipped sliders
- Animated tab bar, blur/shadow effects, rounded rectangles
- Fonts: Inter Variable, JetBrains Mono, Material Icons, FontAwesome
- Notifications, keybinds display, spectator list, watermark, performance overlay

## Architecture

```
ViceCity/
├── core/                    # Core systems
│   ├── entry.cpp           # DLL entry point, initialization
│   ├── hooks/              # VMT/MinHook, D3D11 Present/ResizeBuffers
│   ├── memory/             # Pattern scanner, netvar manager, offset dumper
│   ├── config/             # JSON config manager with schema validation
│   ├── logger/             # Async logging with console/file output
│   └── sdk/                # SDK structs, interfaces, virtual indices
├── features/
│   ├── rage/               # RageBot, exploits, anti-aim, resolver, autowall
│   ├── legit/              # Triggerbot, backtrack, legit AA, movement
│   ├── visuals/            # ESP, chams, world, radar, effects
│   ├── misc/               # Movement, logs, skinchanger, inventory UI
│   └── lua/                # Lua scripting engine (sol2)
├── gui/                    # ImGui-based GUI with ViceCity theme
│   ├── tabs/               # RageBot, LegitBot, Visuals, Misc, Config, Lua
│   └── vice_*.cpp          # Theme, widgets, fonts, rendering
├── resolver/               # Resolver, animation fix, lag compensation
├── skinchanger_system/     # Full inventory/skinchanger implementation
│   ├── inventory_core      # Inventory management
│   ├── case_opening        # Case opening simulation
│   ├── sticker_tool        # Sticker apply/scrape/peel
│   ├── inspect_panel       # 3D inspect panel
│   ├── music_kit_manager   # Music kits/MVP anthems
│   ├── agent_manager       # Agents/patches
│   ├── glove_manager       # Gloves
│   ├── medal_manager       # Medals
│   ├── statrak_manager     # StatTrak spoofing
│   ├── rarity_glow         # Rarity border glow
│   ├── pattern_seed        # Pattern seed browser
│   └── economy_sim         # Fake economy/market
└── build/                  # CMake build system
```

## Build Requirements

- Visual Studio 2022 17.4+
- CMake 3.20+
- Windows 10 SDK 10.0.22621+
- C++20

## Dependencies (included in vendor/)

- **ImGui** - Immediate mode GUI (docking, viewports branch)
- **MinHook** - x64 API hooking library
- **nlohmann/json** - JSON parsing
- **sol2** - Lua 5.4 C++ bindings
- **fmt** - Formatting library

## Building

```powershell
cd build
.\scripts\build_release.ps1
```

Output: `build/bin/Release/ViceCity.dll`

## Usage

1. Inject `ViceCity.dll` into CS2 process
2. Press `INSERT` to open menu
3. Configure features in tabs
4. Configs saved to `%LOCALAPPDATA%/ViceCity/`

## Keybinds

| Key | Action |
|-----|--------|
| INSERT | Toggle menu |
| Custom | Feature keybinds (configure in menu) |

## Configuration

Configs stored in `%LOCALAPPDATA%/ViceCity/`:
- `default.json` - Main config
- `backups/` - Auto backups
- `inventory.json` - Skinchanger inventory
- `sticker_presets.json` - Sticker tool presets
- `lua/scripts/` - Lua scripts

## Lua API

```lua
-- Menu
menu.is_open()
menu.set_open(true)

-- Engine
engine.client_cmd("say hello")
engine.get_local_player()
engine.is_in_game()

-- Entity
entity.get_local_player()
entity.get_player(1)
entity.get_origin(ent)

-- Render
render.world_to_screen({x, y, z})
render.draw_line(x1, y1, x2, y2, color)
render.draw_text("text", x, y, color)

-- Callbacks
callbacks.register("on_create_move", function() end)
callbacks.register("on_render", function() end)

-- Config
config.get("category", "key", default)
config.set("category", "key", value)
```

## Safety

- **No aimbot on LegitBot** - VACNet safe
- **Client-side only** - Skinchanger, visuals, legit features
- **No kernel driver** - User-mode only
- **Manual map** - No auto-injection

## Credits

- Built for educational/research purposes
- Inspired by memesense, neverlose, skeet
- ImGui, MinHook, sol2, nlohmann/json, fmt libraries

## Disclaimer

This software is for educational purposes only. Use at your own risk. The authors are not responsible for any bans, legal issues, or damages resulting from use.

---

**ViceCity v1.0.0** - Built with 💖 by the Void