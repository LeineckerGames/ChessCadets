"""
ChessKids - Decorate the five piece arenas with a CYBERPUNK aesthetic.

RUN ONCE from inside the open Unreal Editor:
    Tools -> Execute Python Script... -> select this file
(If a "Save changes?" dialog pops for the open level, click DON'T SAVE -- this
script loads each map fresh from disk and saves it itself.)

LOOK
----
Dark matte structures (near-black) framed by glowing NEON: trim strips along wall
tops, a neon outline on the floor around the board, tall corner pylons, and a few
colored point lights for mood. Each piece gets its own accent color.

Glow comes from tiny UNLIT EMISSIVE materials generated under /Game/Materials/Neon
(they bloom under the project's Lumen + post FX). If material creation fails for any
reason, the code falls back to a bright base-color tint so the arena is still themed.

Geometry is single SCALED cubes (one actor per wall/tower/strip), so each arena is
only ~15-25 actors. Idempotent: re-run anytime to tweak.

Accent per piece (board sits at origin, spanning -400..+400):
  Pawn    cyan     Knight  green     Bishop  magenta
  Rook    amber    Royalty purple (+ gold gateway)
"""

import unreal

MAPS_DIR = "/Game/Maps"
NEON_DIR = "/Game/Materials/Neon"
CUBE = "/Engine/BasicShapes/Cube.Cube"
BASE_MAT = "/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"
DECO_PREFIX = "Deco_"
STALE_PREFIXES = (DECO_PREFIX, "RoyaltyWall_", "WallBlock", "TowerBlock", "ArchBlock")

DARK = (0.015, 0.015, 0.025, 1.0)           # near-black structure tint

editor_assets = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
_cube = editor_assets.load_asset(CUBE)
_base_mat = editor_assets.load_asset(BASE_MAT)
_neon_cache = {}


def log(m):
    unreal.log("[ChessKids deco] " + str(m))


def warn(m):
    unreal.log_warning("[ChessKids deco] " + str(m))


# ---------------------------------------------------------------------------
# Emissive neon material (cached). Returns a UMaterial or None on failure.
# ---------------------------------------------------------------------------
def neon_material(key, color, intensity=8.0):
    if key in _neon_cache:
        return _neon_cache[key]
    full = NEON_DIR + "/M_Neon_" + key
    try:
        if editor_assets.does_asset_exist(full):
            mat = editor_assets.load_asset(full)
        else:
            mat = asset_tools.create_asset("M_Neon_" + key, NEON_DIR,
                                           unreal.Material, unreal.MaterialFactoryNew())
            mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
            node = unreal.MaterialEditingLibrary.create_material_expression(
                mat, unreal.MaterialExpressionConstant3Vector)
            node.set_editor_property("constant", unreal.LinearColor(
                color[0] * intensity, color[1] * intensity, color[2] * intensity, 1.0))
            unreal.MaterialEditingLibrary.connect_material_property(
                node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            unreal.MaterialEditingLibrary.recompile_material(mat)
            editor_assets.save_asset(full)
    except Exception as exc:
        warn("neon material '{0}' failed, will fall back to tint: {1}".format(key, exc))
        mat = None
    _neon_cache[key] = mat
    return mat


# ---------------------------------------------------------------------------
# Primitive: one scaled cube. Either an emissive `mat` OR a tint `color`.
# ---------------------------------------------------------------------------
def block(label, loc, scale, color=None, mat=None):
    a = actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(*loc), unreal.Rotator(0, 0, 0))
    if not a:
        return None
    a.set_actor_label(DECO_PREFIX + label)
    try:
        a.set_folder_path("Decoration")
    except Exception:
        pass
    smc = a.get_editor_property("StaticMeshComponent")
    if smc and _cube:
        smc.set_static_mesh(_cube)
        try:
            if mat is not None:
                smc.set_material(0, mat)
            elif color is not None and _base_mat:
                mid = smc.create_dynamic_material_instance(0, _base_mat)
                if mid:
                    mid.set_vector_parameter_value("Color", unreal.LinearColor(*color))
        except Exception as exc:
            warn("material set failed ({0}): {1}".format(label, exc))
    a.set_actor_scale3d(unreal.Vector(*scale))
    return a


def neon_block(label, loc, scale, key, color):
    """Glowing cube: emissive material if available, else a bright tint."""
    mat = neon_material(key, color)
    if mat is not None:
        return block(label, loc, scale, mat=mat)
    return block(label, loc, scale, color=(color[0], color[1], color[2], 1.0))


def neon_light(label, loc, color, intensity=40000.0, radius=2000.0):
    try:
        pl = actor_subsystem.spawn_actor_from_class(
            unreal.PointLight, unreal.Vector(*loc), unreal.Rotator(0, 0, 0))
        if not pl:
            return
        pl.set_actor_label(DECO_PREFIX + "Light_" + label)
        try:
            comp = pl.point_light_component
        except Exception:
            comp = pl.get_editor_property("LightComponent")
        if comp:
            comp.set_light_color(unreal.LinearColor(color[0], color[1], color[2], 1.0))
            comp.set_intensity(intensity)
            try:
                comp.set_attenuation_radius(radius)
            except Exception:
                pass
    except Exception as exc:
        warn("light '{0}' failed: {1}".format(label, exc))


# ---------------------------------------------------------------------------
# Composite builders
# ---------------------------------------------------------------------------
def dark_wall_ring(r, h, thick=0.6):
    zc = h * 100 / 2.0
    span = (2 * r) / 100.0 + 1.0
    block("Wall_S", (0, -r, zc), (span, thick, h), color=DARK)
    block("Wall_N", (0,  r, zc), (span, thick, h), color=DARK)
    block("Wall_W", (-r, 0, zc), (thick, span, h), color=DARK)
    block("Wall_E", ( r, 0, zc), (thick, span, h), color=DARK)


def neon_wall_trim(r, h, key, color, thick=0.8):
    """Glowing strip running along the top of each wall."""
    z = h * 100.0
    span = (2 * r) / 100.0 + 1.0
    neon_block("Trim_S", (0, -r, z), (span, thick, 0.18), key, color)
    neon_block("Trim_N", (0,  r, z), (span, thick, 0.18), key, color)
    neon_block("Trim_W", (-r, 0, z), (thick, span, 0.18), key, color)
    neon_block("Trim_E", ( r, 0, z), (thick, span, 0.18), key, color)


def neon_floor_ring(r, key, color):
    """Glowing square outline on the floor around the board."""
    span = (2 * r) / 100.0
    z = 3.0
    neon_block("Floor_S", (0, -r, z), (span, 0.12, 0.05), key, color)
    neon_block("Floor_N", (0,  r, z), (span, 0.12, 0.05), key, color)
    neon_block("Floor_W", (-r, 0, z), (0.12, span, 0.05), key, color)
    neon_block("Floor_E", ( r, 0, z), (0.12, span, 0.05), key, color)


def dark_tower(name, cx, cy, foot, h):
    block("Tower_" + name, (cx, cy, h * 100 / 2.0), (foot, foot, h), color=DARK)


def neon_pylon(name, x, y, h, key, color):
    """Tall thin glowing pillar."""
    neon_block("Pylon_" + name, (x, y, h * 100 / 2.0), (0.4, 0.4, h), key, color)


def dark_column(name, x, y, h, foot=1.0):
    block("Col_" + name, (x, y, h * 100 / 2.0), (foot, foot, h), color=DARK)


# ---------------------------------------------------------------------------
# Per-piece cyberpunk themes
# ---------------------------------------------------------------------------
def theme_pawn():
    key, c = "Cyan", (0.0, 0.85, 1.0)
    dark_wall_ring(700, 2)
    neon_wall_trim(700, 2, key, c)
    neon_floor_ring(500, key, c)
    for nm, x, y in (("a", -700, -700), ("b", 700, -700), ("c", -700, 700), ("d", 700, 700)):
        neon_pylon(nm, x, y, 4, key, c)
    neon_light("p1", (0, 0, 600), c, intensity=30000.0)


def theme_knight():
    key, c = "Green", (0.1, 1.0, 0.25)
    dark_wall_ring(800, 3)
    neon_wall_trim(800, 3, key, c)
    neon_floor_ring(520, key, c)
    neon_pylon("GL", -800, -800, 6, key, c)
    neon_pylon("GR", 800, -800, 6, key, c)
    neon_light("k1", (0, -400, 500), c)
    neon_light("k2", (0, 400, 500), c)


def theme_bishop():
    key, c = "Magenta", (1.0, 0.1, 0.85)
    # Open colonnade: dark pillars with glowing caps.
    pts = []
    for y in range(-600, 601, 300):
        pts += [(-700, y), (700, y)]
    for x in range(-600, 601, 300):
        pts += [(x, -700), (x, 700)]
    seen = set()
    for i, (x, y) in enumerate(pts):
        if (x, y) in seen:
            continue
        seen.add((x, y))
        dark_column("c%d" % i, x, y, 7)
        neon_block("Cap%d" % i, (x, y, 720.0), (1.1, 1.1, 0.2), key, c)
    neon_floor_ring(520, key, c)
    neon_light("b1", (0, 0, 700), c, intensity=45000.0)


def theme_rook():
    key, c = "Amber", (1.0, 0.5, 0.0)
    dark_wall_ring(900, 4)
    neon_wall_trim(900, 4, key, c)
    neon_floor_ring(540, key, c)
    for nm, x, y in (("NE", 900, 900), ("NW", -900, 900), ("SE", 900, -900), ("SW", -900, -900)):
        dark_tower(nm, x, y, 2.5, 10)
        neon_block("Tip_" + nm, (x, y, 1010.0), (2.7, 2.7, 0.2), key, c)
    neon_light("r1", (0, 0, 700), c)


def theme_royalty():
    key, c = "Purple", (0.6, 0.1, 1.0)
    gkey, gold = "Gold", (1.0, 0.8, 0.1)
    dark_wall_ring(900, 4)
    neon_wall_trim(900, 4, key, c)
    neon_floor_ring(540, key, c)
    for nm, x, y in (("NE", 900, 900), ("NW", -900, 900), ("SE", 900, -900), ("SW", -900, -900)):
        dark_tower(nm, x, y, 3.0, 13)
        neon_block("Tip_" + nm, (x, y, 1310.0), (3.2, 3.2, 0.25), key, c)
    # Gold gateway on the south wall.
    neon_pylon("GateL", -250, -900, 7, gkey, gold)
    neon_pylon("GateR", 250, -900, 7, gkey, gold)
    neon_block("Gate_Lintel", (0, -900, 720.0), (5.5, 0.8, 0.3), gkey, gold)
    neon_light("roy1", (0, 0, 800), c, intensity=50000.0)
    neon_light("roy2", (0, -700, 400), gold, intensity=25000.0)


THEMES = [
    ("/Game/Maps/L_Pawn",    theme_pawn),
    ("/Game/Maps/L_Knight",  theme_knight),
    ("/Game/Maps/L_Bishop",  theme_bishop),
    ("/Game/Maps/L_Rook",    theme_rook),
    ("/Game/Maps/L_Royalty", theme_royalty),
]


# ---------------------------------------------------------------------------
def clear_decoration():
    removed = 0
    for a in actor_subsystem.get_all_level_actors():
        try:
            lbl = a.get_actor_label()
        except Exception:
            continue
        if any(lbl.startswith(p) for p in STALE_PREFIXES):
            actor_subsystem.destroy_actor(a)
            removed += 1
    if removed:
        log("  cleared {0} old decoration actor(s)".format(removed))


def save_level_and_actors():
    level_subsystem.save_current_level()
    editor_assets.save_directory(MAPS_DIR, recursive=True, only_if_is_dirty=True)


def main():
    log("Decorating five piece arenas (cyberpunk)...")
    for pkg, builder in THEMES:
        try:
            if not editor_assets.does_asset_exist(pkg):
                warn("missing level (run build_chess_maps.py first): " + pkg)
                continue
            if not level_subsystem.load_level(pkg):
                warn("could not load " + pkg)
                continue
            clear_decoration()
            builder()
            save_level_and_actors()
            log("  decorated + saved " + pkg)
        except Exception as exc:
            warn("error on {0}: {1}".format(pkg, exc))
    log("Done. Re-run anytime; decoration is rebuilt idempotently.")


main()
