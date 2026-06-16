# ChessKids

A remake of the classic ChessKids game built with Unreal Engine 5.7.

## Requirements

- Unreal Engine 5.7
- Python 3.x (optional — for asset generation scripts)

## Getting Started

1. Open `ChessKids.uproject` in Unreal Engine 5.7
2. The project will compile C++ sources and open automatically
3. Open `Content/Maps/L_Field.umap` for the main level

## Project Structure

- `Content/Maps/` — Game levels (`L_Field.umap` is the main field arena)
- `Content/NaturePack/` — Vegetation and nature static meshes
- `Content/Materials/` — Landscape and sky materials
- `Content/GeneratedTextures/` — AI-generated textures (grass, dirt, rock, sky)
- `Content/Data/` — Data assets, including `DA_MapRegistry` (the arena list)
- `Scripts/` — Editor automation (`build_chess_maps.py`)
- `Source/ChessKids/` — C++ game logic
  - `ChessManager` — Chess game state and rules; auto-finds its `Board` at `BeginPlay`
  - `ChessBoard` — Builds the board geometry and square/coordinate helpers
  - `ChessCamera` — Top-down arena camera; auto-finds its `TargetBoard` at `BeginPlay`
  - `ChessKidsGameInstance` — Owns the map registry and performs level travel (`OpenMap`)
  - `ChessMapRegistry` — Data asset describing the selectable arenas
  - `VegetationSpawner` — Procedural foliage/vegetation spawner (drop into nature maps)

## Maps

Each arena is a normal level that contains one chess set: an `AChessBoard`, an
`AChessManager` (with its `Board` reference set), and an `AChessCamera` (with its
`TargetBoard` set). The Manager and Camera also auto-find the Board at runtime, so a
forgotten reference no longer breaks the level.

Map travel is data-driven through `UChessKidsGameInstance` (set as the project's
Game Instance in `Config/DefaultEngine.ini`) and `DA_MapRegistry`
(`/Game/Data/DA_MapRegistry`). The Game Instance loads the registry from that path by
default, so `OpenMap("Field")` works without any Blueprint wiring.

Current / planned arenas:

| Map | Id | Notes |
|-----|----|-------|
| `L_Field` | `Field` | Existing outdoor arena with foliage |
| `L_Studio` | `Studio` | Minimal indoor arena for fast iteration |
| `L_Cyber` | `Cyber` | Cyberpunk arena built from `Content/CyberpunkIndustries` |
| `L_MainMenu` | — | Front-end level hosting the map picker UI |

### Bootstrapping maps and the registry

With the project open in the editor, run `Scripts/build_chess_maps.py` via
**Tools → Execute Python Script**. It creates `DA_MapRegistry`, plus `L_Studio` and
`L_MainMenu`, each pre-populated with a playable, pre-wired chess set, floor, and
lighting. It deliberately skips the design-heavy `L_Cyber` and `WBP_MapSelect`.

### Adding a new map by hand

1. Duplicate `L_Field` (or create a new level) under `Content/Maps/`, prefixed `L_`.
2. Place one `AChessBoard`, one `AChessManager`, and one `AChessCamera` on a flat
   surface. Set the Manager's `Board` and the Camera's `TargetBoard` to that board
   (or rely on the runtime auto-find). Optionally assign piece-mesh overrides on the
   Manager for nicer pieces.
3. Add an entry to `DA_MapRegistry` (`MapId`, `DisplayName`, `Level`).
4. Add the map to the package list in
   `[/Script/UnrealEd.ProjectPackagingSettings]` in `Config/DefaultEngine.ini`
   (uncomment or add a `+MapsToCook=(FilePath="/Game/Maps/L_YourMap")` line).

### Front-end flow (to author in the editor)

- `WBP_MapSelect`: reads `DA_MapRegistry` and calls
  `GetGameInstance -> Cast to ChessKidsGameInstance -> OpenMap(MapId)` per button.
- `L_MainMenu`: a thin level whose HUD shows `WBP_MapSelect`.
- `WBP_GameOver` can reuse the same `OpenMap` call for rematch / change-arena.
- Once `L_MainMenu` exists, point `EditorStartupMap` and `GameDefaultMap` in
  `Config/DefaultEngine.ini` at it (currently set to `L_Field`).

## Features

- Procedural field environment with sculpted landscape and foliage
- AI-generated textures via Stable Diffusion
- 3-layer auto-blend landscape material (grass/dirt/rock)
- C++ procedural vegetation spawner (`AVegetationSpawner`)
- Data-driven multi-map support with a simple map picker flow
- Chess board environment (in progress)
- Chess engine (in progress)
