"""
ChessKids - Piece-themed map bootstrap (UE5.7 Python).

HOW TO RUN
----------
Run this once from inside the open Unreal Editor for the ChessKids project:

    Tools -> Execute Python Script... -> select this file

(Alternatively paste the path into the Output Log "Cmd" box with the dropdown
set to "Python", or call:  py "D:/unrealprojects/chesskids/Scripts/build_chess_maps.py")

It needs no command-line arguments and never prompts for input.

WHAT IT DOES (each step is independent; one failure does not abort the rest)
---------------------------------------------------------------------------
  1. Creates / REPLACES /Game/Data/DA_MapRegistry (a UChessMapRegistry) so that
     its "Maps" array holds EXACTLY the five piece-themed arenas defined in
     MAP_SPECS below. Any previously authored entries (e.g. Field / Studio /
     Cyber) are dropped -- this is load-and-overwrite if the asset already
     exists, create-then-fill if it does not.
  2. Creates the five piece levels under /Game/Maps. Each level is populated
     with a pre-wired chess set (board + manager + camera) and has its board
     CurrentThemeIndex stamped 0..4 to match the map's themeIndex, so the
     arenas are visually distinguishable once BoardThemes are authored. While
     BoardThemes is empty this is a harmless no-op (AChessBoard::OnConstruction
     -> SetBoardTheme safely ignores an out-of-range/empty theme list).

King and Queen are intentionally merged into a single "Royalty" map, since they
are the inseparable heart of the game.

The chess actors (AChessBoard / AChessManager / AChessCamera) are placed directly
and pre-wired (Manager.Board and Camera.TargetBoard) so each map is ready to play
with no further setup. NOTE: the actual chess PIECES are spawned by
AChessManager::SpawnAllPieces(), which runs from BeginPlay() -- so the saved level
contains only the board/manager/camera/lighting; the pieces appear at PIE/runtime,
not in the saved level. Pieces use their built-in primitive meshes unless mesh
overrides (WhitePieceMeshes / BlackPieceMeshes) are assigned on the Manager later.

WORLD PARTITION / EXTERNAL ACTORS
---------------------------------
Since UE5.1 the default level template is a World Partition level, in which every
placed actor is stored in its OWN package under Content/__ExternalActors__/...
(this repo already shows that pattern under .../Maps/L_Field/). Saving only the
map package (LevelEditorSubsystem.save_current_level()) does NOT reliably flush
those dirty per-actor packages, which can persist a level with ZERO actors. To
avoid that, after populating an arena we save the map AND then flush every dirty
package under /Game/Maps via EditorAssetLibrary.save_directory(...). See
save_level_and_actors() below.

IDEMPOTENCY
-----------
Level creation is idempotent: an existing level is left untouched (detected via
EditorAssetLibrary.does_asset_exist) and is therefore NOT re-stamped with its
theme index on a re-run. The registry, however, is ALWAYS rewritten to match this
spec so it can never drift out of sync with the level set. To force a level to be
rebuilt/re-themed, delete it first and re-run.

FOLLOW-UP (manual)
------------------
After running, add the five L_<Piece> maps to the packaged map list in
Config/DefaultEngine.ini, and assign DA_MapRegistry on your
ChessKidsGameInstance (MapRegistry property).
"""

import unreal

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
MAPS_DIR = "/Game/Maps"
DATA_DIR = "/Game/Data"
REGISTRY_NAME = "DA_MapRegistry"
REGISTRY_PATH = DATA_DIR + "/" + REGISTRY_NAME

# ---------------------------------------------------------------------------
# Single source of truth for the five piece-themed maps.
# Edit ONLY this list to change the map set.
#   (map_id, display_name, level_package, level_object_path, theme_index, unlocked)
# ---------------------------------------------------------------------------
MAP_SPECS = [
    ("Pawn",    "The Pawn",               MAPS_DIR + "/L_Pawn",    "/Game/Maps/L_Pawn.L_Pawn",       0, True),
    ("Knight",  "The Knight",             MAPS_DIR + "/L_Knight",  "/Game/Maps/L_Knight.L_Knight",   1, True),
    ("Bishop",  "The Bishop",             MAPS_DIR + "/L_Bishop",  "/Game/Maps/L_Bishop.L_Bishop",   2, True),
    ("Rook",    "The Rook",               MAPS_DIR + "/L_Rook",    "/Game/Maps/L_Rook.L_Rook",       3, True),
    ("Royalty", "Royalty - King & Queen", MAPS_DIR + "/L_Royalty", "/Game/Maps/L_Royalty.L_Royalty", 4, True),
]

# ---------------------------------------------------------------------------
# Editor subsystems / libraries (these match what the existing tooling uses).
# EditorAssetLibrary is the legacy static-class API; it still functions in 5.7
# (it may emit deprecation noise). We keep it to match the existing tooling and
# because we rely on save_directory(), which it exposes.
# ---------------------------------------------------------------------------
editor_assets = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


# ---------------------------------------------------------------------------
# Logging helpers
# ---------------------------------------------------------------------------
def log(msg):
    unreal.log("[ChessKids maps] " + str(msg))


def warn(msg):
    unreal.log_warning("[ChessKids maps] " + str(msg))


# ---------------------------------------------------------------------------
# Spawning
# ---------------------------------------------------------------------------
def spawn(actor_class, location, rotation=None):
    """Spawn an actor of actor_class at location (optionally rotation) in the
    current level and return it."""
    rotation = rotation or unreal.Rotator(0.0, 0.0, 0.0)
    return actor_subsystem.spawn_actor_from_class(actor_class, location, rotation)


def populate_arena(theme_index, with_floor=True):
    """Place a pre-wired chess set (+ lighting and an optional floor) into the
    current level and stamp the board with this map's theme index.

    Only the board / manager / camera / lighting / floor are placed here; the
    chess pieces themselves are spawned by AChessManager::SpawnAllPieces(),
    which runs from BeginPlay -- so they appear at PIE/runtime, not in the
    saved level.

    Returns the spawned AChessBoard.
    """
    # Prefer the configured BP_ChessBoard (it carries the translucent holographic
    # materials, square materials and neon edge lights). Fall back to the raw C++
    # class only if the Blueprint is missing -- the raw class has null materials
    # and renders the opaque /Engine WorldGridMaterial, covering the board.
    board_class = unreal.load_class(None, "/Game/Blueprints/BP_ChessBoard.BP_ChessBoard_C")
    if not board_class:
        warn("BP_ChessBoard not found; using raw C++ ChessBoard (no holo materials).")
        board_class = unreal.ChessBoard
    board = spawn(board_class, unreal.Vector(0.0, 0.0, 0.0))
    # Stamp this map's theme so arenas are visually distinguishable once
    # BoardThemes are authored. Harmless no-op while BoardThemes is empty:
    # AChessBoard::OnConstruction -> SetBoardTheme(CurrentThemeIndex) guards it
    # with BoardThemes.IsValidIndex(...).
    board.set_editor_property("CurrentThemeIndex", int(theme_index))
    # Also drive SetBoardTheme so the persisted index and any future material
    # swap stay in lockstep (no-op today while BoardThemes is empty; the
    # UFUNCTION re-guards the index internally via IsValidIndex).
    try:
        board.set_board_theme(int(theme_index))
    except Exception as exc:
        warn("set_board_theme({0}) unavailable/failed (index still stamped): {1}".format(theme_index, exc))

    manager = spawn(unreal.ChessManager, unreal.Vector(0.0, 0.0, 0.0))
    manager.set_editor_property("Board", board)

    camera = spawn(unreal.ChessCamera, unreal.Vector(0.0, 0.0, 0.0))
    camera.set_editor_property("TargetBoard", board)

    # Basic lighting / atmosphere so the arena is immediately viewable.
    spawn(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 1000.0),
          unreal.Rotator(-50.0, -45.0, 0.0))
    spawn(unreal.SkyLight, unreal.Vector(0.0, 0.0, 800.0))
    spawn(unreal.SkyAtmosphere, unreal.Vector(0.0, 0.0, 0.0))

    if with_floor:
        floor = spawn(unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, -5.0))
        plane = editor_assets.load_asset("/Engine/BasicShapes/Plane")
        if plane:
            # Use the reflected property accessor for the mesh component.
            smc = floor.get_editor_property("StaticMeshComponent")
            if smc:
                smc.set_static_mesh(plane)
            else:
                warn("StaticMeshActor has no StaticMeshComponent; floor left without a mesh.")
        else:
            warn("Could not load /Engine/BasicShapes/Plane; floor left without a mesh.")
        floor.set_actor_scale3d(unreal.Vector(40.0, 40.0, 1.0))

    return board


# ---------------------------------------------------------------------------
# Saving (World-Partition safe)
# ---------------------------------------------------------------------------
def save_level_and_actors():
    """Save the current level AND flush any dirty external-actor packages.

    In a World Partition level the placed actors live in separate packages under
    __ExternalActors__; save_current_level() alone does not reliably write them,
    so we follow up with a directory save over /Game/Maps to flush every dirty
    per-actor package and guarantee the arena is actually persisted.
    """
    level_subsystem.save_current_level()
    # Flush dirty external-actor packages (and the map itself) under /Game/Maps.
    editor_assets.save_directory(MAPS_DIR, recursive=True, only_if_is_dirty=True)


# ---------------------------------------------------------------------------
# Level creation (idempotent)
# ---------------------------------------------------------------------------
def create_level(package_path, theme_index, with_floor=True):
    """Create one piece-themed level. Idempotent: skips if the asset exists.

    NOTE: a skipped (already existing) level is NOT re-stamped with theme_index,
    per the stated idempotency rule. Delete the level to force a rebuild.
    """
    try:
        if editor_assets.does_asset_exist(package_path):
            warn("Level already exists, skipping (not re-themed): " + package_path)
            return
        if not level_subsystem.new_level(package_path):
            warn("Failed to create level: " + package_path)
            return
        populate_arena(theme_index, with_floor=with_floor)
        save_level_and_actors()
        log("Created and saved level: {0} (themeIndex={1})".format(package_path, theme_index))
    except Exception as exc:  # report and continue with the remaining steps
        warn("Error creating level {0}: {1}".format(package_path, exc))


# ---------------------------------------------------------------------------
# Registry entries
# ---------------------------------------------------------------------------
def make_entry(map_id, display, level_object_path, unlocked=True):
    """Build a single FChessMapEntry (unreal.ChessMapEntry).

    Thumbnail is intentionally left at its default (None).
    """
    entry = unreal.ChessMapEntry()
    entry.set_editor_property("MapId", map_id)                                    # FName <- bare str (do NOT wrap)
    entry.set_editor_property("DisplayName", unreal.Text.from_string(display))    # FText
    entry.set_editor_property("Level", unreal.SoftObjectPath(level_object_path))  # TSoftObjectPtr<UWorld>
    entry.set_editor_property("bUnlockedByDefault", bool(unlocked))               # bool
    return entry


def create_registry():
    """REPLACE semantics for /Game/Data/DA_MapRegistry.

    If it already exists, load it and overwrite its "Maps" array with exactly the
    five piece-themed entries (dropping any old Field / Studio / Cyber entries).
    If it does not exist, create it as a UChessMapRegistry and then fill it.
    Either way, save the asset afterward (forced, regardless of dirty flag).
    """
    try:
        if editor_assets.does_asset_exist(REGISTRY_PATH):
            registry = editor_assets.load_asset(REGISTRY_PATH)
            if not registry:
                warn("DA_MapRegistry exists but could not be loaded; aborting registry step.")
                return
            log("DA_MapRegistry found - overwriting its Maps array.")
        else:
            registry = asset_tools.create_asset(
                REGISTRY_NAME, DATA_DIR, unreal.ChessMapRegistry, unreal.DataAssetFactory())
            if not registry:
                warn("Could not create DA_MapRegistry "
                     "(create it manually as a UChessMapRegistry under /Game/Data).")
                return
            log("Created new DA_MapRegistry.")

        entries = [make_entry(mid, disp, obj_path, unlocked)
                   for (mid, disp, _pkg, obj_path, _theme, unlocked) in MAP_SPECS]
        # Wholesale assignment overwrites any prior Field/Studio/Cyber entries.
        registry.set_editor_property("Maps", entries)
        # Force the save so we never depend on dirty-flag propagation.
        editor_assets.save_asset(REGISTRY_PATH, only_if_is_dirty=False)
        log("DA_MapRegistry now holds {0} entries: {1}.".format(
            len(entries), ", ".join(s[0] for s in MAP_SPECS)))
    except Exception as exc:  # report and continue
        warn("Error writing DA_MapRegistry: {0}".format(exc))


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
def main():
    log("Starting piece-themed map bootstrap (5 maps; King + Queen merged into Royalty)...")

    # 1. Registry is ALWAYS rewritten to exactly the five entries.
    create_registry()

    # 2. Levels are created idempotently, each stamped with its theme index.
    for (_mid, _disp, pkg, _obj_path, theme_index, _unlocked) in MAP_SPECS:
        create_level(pkg, theme_index, with_floor=True)

    log("Done. Five piece-themed maps (Pawn / Knight / Bishop / Rook / Royalty) "
        "are registered in DA_MapRegistry. Next: add the L_<Piece> maps to the "
        "packaged map list in Config/DefaultEngine.ini and assign DA_MapRegistry "
        "to your ChessKidsGameInstance.")


main()