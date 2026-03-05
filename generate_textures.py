"""
generate_textures.py
Generates holographic/cyberpunk chess textures for ChessKids.
Output: Content/GeneratedTextures/
"""

from PIL import Image, ImageDraw, ImageFilter
import math, os

OUT = "Content/GeneratedTextures"
os.makedirs(OUT, exist_ok=True)

SIZE = 512


def save(img, name):
    path = os.path.join(OUT, name)
    img.save(path)
    print(f"  saved {path}")


# ---------------------------------------------------------------
# T_GridOverlay — cyan neon grid lines on near-black background
# ---------------------------------------------------------------
def make_grid_overlay():
    img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 8, 255))
    draw = ImageDraw.Draw(img)

    cell = SIZE // 8  # 8x8 grid matching the chess board
    bright = (0, 230, 255, 255)
    dim    = (0, 120, 160, 180)

    for i in range(9):
        x = i * cell
        w = 3 if i % 8 == 0 else 1
        c = bright if i % 8 == 0 else dim
        draw.line([(x, 0), (x, SIZE)], fill=c, width=w)
        draw.line([(0, x), (SIZE, x)], fill=c, width=w)

    # soft glow pass
    glow = img.filter(ImageFilter.GaussianBlur(2))
    img  = Image.blend(img, glow, 0.4)
    save(img, "T_GridOverlay.png")


# ---------------------------------------------------------------
# T_ScanLine — bright horizontal band, fades top and bottom
# Used by the scan plane that sweeps across the board
# ---------------------------------------------------------------
def make_scan_line():
    img  = Image.new("RGBA", (SIZE, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    cy   = 32

    for y in range(64):
        dist  = abs(y - cy)
        alpha = max(0, 255 - int(dist * 14))
        r = 0
        g = int(200 * (alpha / 255))
        b = int(255 * (alpha / 255))
        draw.line([(0, y), (SIZE, y)], fill=(r, g, b, alpha))

    save(img, "T_ScanLine.png")


# ---------------------------------------------------------------
# T_LightSquare — pale holographic tile, slight inner glow
# ---------------------------------------------------------------
def make_light_square():
    img  = Image.new("RGBA", (SIZE, SIZE), (10, 30, 50, 255))
    draw = ImageDraw.Draw(img)

    # base subtle gradient from center
    cx = cy = SIZE // 2
    for y in range(SIZE):
        for x in range(SIZE):
            d = math.sqrt((x - cx)**2 + (y - cy)**2) / (SIZE * 0.7)
            v = max(0, 1 - d)
            r = int(40  + v * 80)
            g = int(140 + v * 80)
            b = int(180 + v * 60)
            img.putpixel((x, y), (r, g, b, 255))

    # thin glowing border
    draw.rectangle([0, 0, SIZE-1, SIZE-1], outline=(0, 220, 255, 200), width=2)
    draw.rectangle([3, 3, SIZE-4, SIZE-4], outline=(0, 180, 220, 100), width=1)

    img = img.filter(ImageFilter.GaussianBlur(1))
    save(img, "T_LightSquare.png")


# ---------------------------------------------------------------
# T_DarkSquare — deep blue holographic tile
# ---------------------------------------------------------------
def make_dark_square():
    img  = Image.new("RGBA", (SIZE, SIZE), (2, 8, 20, 255))
    draw = ImageDraw.Draw(img)

    cx = cy = SIZE // 2
    for y in range(SIZE):
        for x in range(SIZE):
            d = math.sqrt((x - cx)**2 + (y - cy)**2) / (SIZE * 0.7)
            v = max(0, 1 - d) * 0.4
            r = int(5  + v * 20)
            g = int(20 + v * 60)
            b = int(60 + v * 80)
            img.putpixel((x, y), (r, g, b, 255))

    draw.rectangle([0, 0, SIZE-1, SIZE-1], outline=(0, 100, 160, 160), width=2)

    img = img.filter(ImageFilter.GaussianBlur(1))
    save(img, "T_DarkSquare.png")


# ---------------------------------------------------------------
# T_EdgeGlow — radial glow used as a sprite for edge lights
# ---------------------------------------------------------------
def make_edge_glow():
    img = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
    cx = cy = 128
    for y in range(256):
        for x in range(256):
            d = math.sqrt((x - cx)**2 + (y - cy)**2) / 128.0
            a = max(0, 1 - d)
            a = int(a ** 2 * 255)
            img.putpixel((x, y), (0, int(180 * a/255), 255, a))
    save(img, "T_EdgeGlow.png")


if __name__ == "__main__":
    print("Generating textures...")
    make_grid_overlay()
    make_scan_line()
    make_light_square()
    make_dark_square()
    make_edge_glow()
    print("Done.")
