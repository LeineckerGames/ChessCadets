# ChessKids

A kids' chess game built in **Unreal Engine 5.7**. You play White against a built‑in
chess AI on a holographic board, set inside neon, cyberpunk‑styled arenas. Each chess
piece has its own themed arena, selectable from an in‑game menu.

The chess rules and AI are real C++ — the project embeds the open‑source **Pulse** chess
engine and drives it from the gameplay code, so moves are fully validated and the
computer actually searches for replies.

---

## Requirements

**To run the packaged game / play in editor**

- **Windows 10/11 (64‑bit).** The project targets Direct3D 12 (Shader Model 6).
- A **DirectX 12 GPU with hardware ray tracing** (e.g. NVIDIA RTX 20‑series or AMD RX
  6000‑series and newer). The project enables Lumen global illumination & reflections,
  hardware ray tracing, Substrate materials, and Virtual Shadow Maps — an older
  DX11‑only GPU will not render it correctly.
- Otherwise, Epic's published
  [hardware & software specifications for Unreal Engine 5](https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine)
  apply.

**To build / develop**

- **Unreal Engine 5.7** (the `.uproject` is bound to engine association `5.7`).
- **Visual Studio 2022** with the *Game development with C++* workload.
- **Git** and **[Git LFS](https://git-lfs.com/)** — all `.uasset`/`.umap` binaries are
  stored in LFS, so the repo is unusable without it.
- **Python 3** is *optional* and only needed if you run the editor automation scripts in
  `Scripts/` (they execute through Unreal's built‑in Python).

---

## Getting Started

```bash
git lfs install                 # one-time per machine
git clone https://github.com/LeineckerGames/ChessCadets.git
cd ChessCadets
git lfs pull                    # download the binary assets
```

1. Right‑click `ChessKids.uproject` → **Generate Visual Studio project files** (or just
   open it — the editor offers to build the `ChessKids` module).
2. Open `ChessKids.uproject`. If prompted, **build** the C++ module.
3. Press **Play**. The game boots into `MainMenu` (configured as the startup/default map).

Navigation flow: **MainMenu → LevelSelect → a piece arena**.

---

## How it plays

- The board, pieces, camera and rules are spawned and wired automatically — drop the
  actors in a level (or use the bootstrap script) and it just runs.
- You control **White**. Hover a square to highlight it, click one of your pieces to
  select it (legal targets light up), then click a target to move.
- After your move, the AI replies on a short delay. Difficulty controls how deeply the
  engine searches (see below).
- Checkmate, stalemate, threefold repetition and insufficient‑material draws are all
  detected; the result drives the Game Over widget.

---

## Project Structure

### C++ (`Source/ChessKids/`)

| Class | Role |
|-------|------|
| `AChessManager` | Central game logic. Wraps the Pulse engine, validates/applies moves, drives the AI, detects game over, and spawns/moves the visual pieces. Exposes `OnMoveMade`, `OnAIMoveReady`, `OnGameOver` events. |
| `AChessBoard` | Procedurally builds the 8×8 board (64 square + 64 hidden highlight meshes), a holographic grid overlay, an animated scan plane and neon edge lights. Owns coordinate ↔ square conversion, highlighting, actor snapping, and swappable per‑arena color **themes**. |
| `AChessPiece` | One piece. Uses a **skeletal mesh** (per‑type model + dynamic color tint), configurable via a `FPieceMeshConfig` override. |
| `AChessPlayerController` | Mouse hover/selection input. Resolves the board square under the cursor, highlights legal moves, submits moves, and schedules the AI reply. |
| `AChessCamera` | Fixed spring‑arm camera that frames the board (configurable arm length / pitch / yaw). |
| `UChessKidsGameInstance` | Set as the project Game Instance. Owns the arena registry and performs level travel via `OpenMap(MapId)`. |
| `UChessMapRegistry` | `UPrimaryDataAsset` (authored as `DA_MapRegistry`) listing the selectable arenas (`FChessMapEntry`: id, display name, level, thumbnail, unlocked flag). |
| `AVegetationSpawner` | Procedurally scatters static‑mesh foliage — or an overridden actor class such as a hover‑car — around itself via ground line‑traces, excluding a radius around the board. Re‑randomizes every time the level starts. |

The Pulse chess engine source is embedded under `Source/ChessKids/Engine/` (see
[The chess AI](#the-chess-ai)).

### Content (`Content/`)

| Path | Contents |
|------|----------|
| `Maps/` | Arenas and front‑end levels (see [Maps](#maps)). |
| `Blueprints/` | `BP_ChessBoard` (configured board), `BP_OrbitCar` (cyberpunk hover‑car prop), `BP_KingNPC` (waving King NPC), `ChessGameMode`, `WBP_GameOver`. |
| `Blueprints/Models/` | The six chess pieces as skeletal‑mesh sets (mesh + skeleton + physics asset + anim): `pawn__2_`, `rook`, `knight`, `bishop`, `queen`, `king`. |
| `Materials/Board/` | Board surface & highlight materials: `M_LightSquare`, `M_DarkSquare`, `M_HoloGrid`, `M_Selection`, `M_LegalMove`, `M_Hover`, `M_CityFloor`. |
| `UI/` | `WBP_MainMenu`, `WBP_LevelSelect`, `M_BuildingSilhouette` (skyline backdrop). |
| `Data/` | `DA_MapRegistry` — the arena list consumed by the menu and `OpenMap`. |
| `CyberpunkIndustries/`, `Fab/` | Third‑party city kit and marketplace dressing (buildings, flying/parked cars, kiosks, rooftops). |
| `NaturePack/` | Foliage library used by `AVegetationSpawner` to populate `L_Field`. |
| `GeneratedTextures/` | Generated texture outputs (AI sky/ground for `L_Field`; procedural holographic board textures). |
| `Sky/`, `Meshy_AI*` | HDRI sky lighting; the AI‑generated character mesh behind `BP_KingNPC`. |

---

## The chess AI

The project embeds the **[Pulse](https://github.com/fluxroot/pulse) chess engine** by
Phokham Nonava (MIT‑licensed), adapted for UE5. Its source lives in
`Source/ChessKids/Engine/` (plus the `model/` subfolder) and is compiled into the game
module — there is no external dependency.

- **What's inside:** a 0x88 board `Position` with bitboards and Zobrist hashing,
  `MoveGenerator` (legal moves incl. castling / en passant / promotion), a static
  `evaluation` (material + mobility), and an iterative‑deepening alpha‑beta `Search`
  with quiescence and MVV‑LVA move ordering. The original engine's threading is replaced
  with Unreal's `FRunnableThread`/`FEvent`.
- **Integration:** `AChessManager` holds a private `FEngineImpl : pulse::Protocol` that
  owns the `Position` and `Search`. The search runs on a worker thread and reports its
  result through `sendBestMove`; the callback marshals back to the game thread with
  `AsyncTask` (guarded by a `TWeakObjectPtr`) before applying the move.
- **Build flags:** `ChessKids.Build.cs` compiles the engine with `bEnableExceptions`,
  `bUseRTTI`, `CppStandard = Cpp20`, `bUseUnity = false`, `PCHUsage = NoPCHs`, and adds
  `Engine/` to the private include path so the engine headers resolve by bare name.

### Difficulty

`AChessManager::SetDifficulty(Level)` maps a level to the engine's `AISearchDepth`:

| Level | Search depth | Behavior |
|-------|--------------|----------|
| 1 (Easy) | 1 | Shallow search, **plus a 50% chance to play a purely random legal move** |
| 2 (Medium) | 3 | Straight engine search |
| 3 (Hard) | 6 | Deeper engine search |

Play currently starts on Easy (`BeginPlay` → `SetDifficulty(1)`). `AISearchDepth` is also
an editable property (clamped 1–20).

---

## Maps

| Map | Purpose |
|-----|---------|
| `MainMenu` | Front‑end entry level (startup & default map). Hosts `WBP_MainMenu`. |
| `LevelSelect` | Arena picker. Hosts `WBP_LevelSelect`, which reads `DA_MapRegistry` and calls `OpenMap`. |
| `L_Pawn` | Piece arena — cyan theme. |
| `L_Knight` | Piece arena — green theme. |
| `L_Bishop` | Piece arena — magenta theme (open colonnade). |
| `L_Rook` | Piece arena — amber theme (corner towers). |
| `L_Royalty` | Combined King + Queen arena — purple theme with a gold gateway. |
| `L_Field` | Open‑world nature/landscape level (World Partition, NaturePack foliage). Separate from the piece arenas. |
| `L_GameOverTest` | Scratch level for exercising the game‑over flow. |

**Navigation:** at startup the Game Instance loads `DA_MapRegistry` and seeds the unlocked
set. `WBP_LevelSelect` lists those entries; choosing one calls
`UChessKidsGameInstance::OpenMap(MapId)`, which resolves the entry's level and travels to it.

### Adding a new arena by hand

1. Create a level under `Content/Maps/` (prefix `L_`).
2. Place one `BP_ChessBoard` (or `AChessBoard`), one `AChessManager`, and one
   `AChessCamera`. References auto‑resolve at runtime, but you can set them explicitly.
3. Add an `FChessMapEntry` to `DA_MapRegistry` (`MapId`, `DisplayName`, `Level`).
4. If it should be packaged, add it to `MapsToCook` in `Config/DefaultEngine.ini`.

---

## Editor automation (`Scripts/`)

Run from the open editor via **Tools → Execute Python Script**:

- **`build_chess_maps.py`** — (re)writes `DA_MapRegistry` to the five entries
  (Pawn/Knight/Bishop/Rook/Royalty) and creates the five `L_<Piece>` levels, each
  pre‑wired with a board + manager + camera, lighting, and a floor. Idempotent; skips
  levels that already exist. Pieces are spawned at runtime, not saved into the level.
- **`decorate_maps.py`** — dresses the five arenas with the cyberpunk look: dark wall
  rings/towers/columns, emissive neon trim and floor outlines, pylons and colored point
  lights (one accent color per piece). Run *after* `build_chess_maps.py`.

---

## Notable project settings (`Config/DefaultEngine.ini`)

- **Maps:** `EditorStartupMap` / `GameDefaultMap` = `MainMenu`;
  `GlobalDefaultGameMode` = `ChessGameMode_C`;
  `GameInstanceClass` = `ChessKidsGameInstance`.
- **Cooked maps:** `MainMenu`, `LevelSelect`, `L_Field`.
- **Rendering:** Lumen GI + reflections, hardware ray tracing, Substrate, Virtual Shadow
  Maps, mesh distance fields; static lighting disabled. RHI = D3D12 / SM6.
- **Input:** Enhanced Input system.
- **Python:** remote execution enabled (`bRemoteExecution=True`) for editor scripting.
- **Plugins:** `ModelingToolsEditorMode` (on), `HDRIBackdrop` (on),
  `VisualStudioTools` (off).

---

## Working with the repository

- `main` is the integrated baseline — branch your work off it and open a PR; **don't push
  directly to `main`.**
- Always have **Git LFS** installed before cloning/pulling, or the binary assets will be
  broken pointers.

---

## Attribution

- **Pulse chess engine** — © 2013‑2023 Phokham Nonava, MIT license
  (`Source/ChessKids/Engine/`).
- **CyberpunkIndustries**, **Fab** packs, **NaturePack**, and the **Meshy AI** character
  are third‑party assets used for environment/character dressing under their respective
  licenses.

> Note: the Pulse source headers reference an MIT `LICENSE` file that is not yet checked
> into this repo — it should be added to fully satisfy the MIT attribution requirement.
