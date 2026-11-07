#!/usr/bin/env python3
"""Draws Kingside's castle screens (title, main menu, lesson page, board) at
160x144 and cuts each into Game Boy Color tiles: every 8x8 cell must use at
most four colours from one of the screen's eight palettes. Writes a C file
per screen for the loader ROMs in mockup_rom/, and a 3x PNG preview.

Light comes from the upper left; forms take three tones with shadows shifted
toward blue. Pieces, board labels and squares come from roupiks' Super Game
Boy Chess asset pack (assets/chess); text uses Not Jam fonts (assets/fonts,
CC0).

    python3 tools/castle_art.py OUTDIR
"""
from __future__ import annotations
import random
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

GAME = Path(__file__).resolve().parent.parent
ATLAS = Image.open(GAME / "assets" / "chess" / "atlas.png").convert("RGBA")
PINK, ORANGE, PURPLE = (233, 181, 163), (187, 117, 71), (64, 51, 83)
W, H = 160, 144


def ttf(name, size):
    return ImageFont.truetype(str(GAME / "assets" / "fonts" / name), size)


LOGO19 = ttf("NotJamThirdDimension19.ttf", 19)
MENU = ttf("NotJamMonoOldPeculiar11.ttf", 11)
SMALL = ttf("UndeadPixel8.ttf", 8)


def rgb(h):
    h = h.lstrip("#")
    return tuple(int(h[i:i + 2], 16) for i in (0, 2, 4))


def mix(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


class Scene:
    def __init__(self, fill):
        self.im = Image.new("RGB", (W, H), fill)
        self.d = ImageDraw.Draw(self.im)
        self.d.fontmode = "1"

    def text(self, x, y, s, f, fill):
        """Ink's top-left at (x, y)."""
        bx0, by0, _, _ = self.d.textbbox((0, 0), s, font=f)
        self.d.text((x - bx0, y - by0), s, font=f, fill=fill)

    def ink_size(self, s, f):
        bx0, by0, bx1, by1 = self.d.textbbox((0, 0), s, font=f)
        return bx1 - bx0, by1 - by0

    def text_center(self, x0, x1, y_mid, s, f, fill):
        w, h = self.ink_size(s, f)
        self.text((x0 + x1 + 1 - w) // 2, y_mid - h // 2, s, f, fill)

    def fits(self, s, f, x, x_max):
        w, _ = self.ink_size(s, f)
        assert x + w - 1 <= x_max, (s, x + w - 1, x_max)

    def rows(self, x0, y0, rows, colours):
        for y, r in enumerate(rows):
            for x, c in enumerate(r):
                if c in colours:
                    self.im.putpixel((x0 + x, y0 + y), colours[c])


# ---- pack art -------------------------------------------------------------

def atlas_cell(x0, y0, w, h):
    out = []
    for y in range(h):
        row = []
        for x in range(w):
            r, g, b, a = ATLAS.getpixel((x0 + x, y0 + y))
            row.append(None if a == 0 else {PINK: "p", ORANGE: "o", PURPLE: "u"}[(r, g, b)])
        out.append(row)
    return out


def ring_split(rows):
    """Pink that touches the outside is the outline ring ('r'); pink inside
    the piece is a highlight ('h')."""
    out = []
    for y, r in enumerate(rows):
        row = []
        for x, c in enumerate(r):
            if c == "p":
                edge = any(not (0 <= y + dy < len(rows) and 0 <= x + dx < len(r)) or rows[y + dy][x + dx] is None
                           for dy, dx in ((-1, 0), (1, 0), (0, -1), (0, 1)))
                row.append("r" if edge else "h")
            else:
                row.append(c)
        out.append(row)
    return out


PIECES = {}
for i, n in enumerate("KQRBNP"):
    PIECES[n] = ring_split(atlas_cell(16 + 16 * i, 224, 16, 16))
    PIECES[n.lower()] = ring_split(atlas_cell(16 + 16 * i, 208, 16, 16))
LETTERS = [atlas_cell(20 + 16 * i, 164, 8, 8) for i in range(8)]
DIGITS = [atlas_cell(20 + 16 * i, 179, 8, 9) for i in range(8)]


def compile_scene(sc, pals, sprites, out):
    im = sc.im
    words = []
    for p in pals:
        assert len(p) == 4 and len(set(p)) == 4, p
        for c in p:
            r, g, b = c
            words.append((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10))
    tiles, index, tmap, amap, bad = [], {}, [], [], []
    for cy in range(18):
        for cx in range(20):
            cols = {im.getpixel((cx * 8 + x, cy * 8 + y)) for y in range(8) for x in range(8)}
            choice = next((i for i, p in enumerate(pals) if cols <= set(p)), None)
            if choice is None:
                bad.append((cx, cy, sorted(cols)))
                tmap.append(0)
                amap.append(0)
                continue
            p = pals[choice]
            t = []
            for y in range(8):
                lo = hi = 0
                for x in range(8):
                    v = p.index(im.getpixel((cx * 8 + x, cy * 8 + y)))
                    lo |= (v & 1) << (7 - x)
                    hi |= (v >> 1) << (7 - x)
                t += [lo, hi]
            t = tuple(t)
            if t not in index:
                index[t] = len(tiles)
                tiles.append(t)
            i = index[t]
            tmap.append(i % 256)
            amap.append(choice | (0x08 if i >= 256 else 0))
    if bad:
        for b in bad[:12]:
            print("  cell", b[:2], "colours", ["#%02X%02X%02X" % c for c in b[2]], file=sys.stderr)
        raise SystemExit(f"{out.name}: {len(bad)} cells fit no palette")
    assert len(tiles) <= 512
    spr = []
    for kind, sq in sprites:
        f, r = "abcdefgh".index(sq[0]), int(sq[1]) - 1
        spr.append((0 if kind == "cursor" else 1, 16 + 16 * f, 8 + 16 * (7 - r)))
    flat = [b for t in tiles for b in t]
    out.write_text("\n".join([
        '#include <stdint.h>', '#include "screen.h"',
        "const uint16_t screen_tile_count = %d;" % len(tiles),
        "const uint8_t screen_tiles[] = {" + ",".join(map(str, flat)) + "};",
        "const uint8_t screen_map[] = {" + ",".join(map(str, tmap)) + "};",
        "const uint8_t screen_attr[] = {" + ",".join(map(str, amap)) + "};",
        "const uint16_t screen_pal[] = {" + ",".join(map(str, words)) + "};",
        "const uint8_t screen_sprite_count = %d;" % len(spr),
        "const uint8_t screen_sprites[] = {" + (",".join("%d,%d,%d" % x for x in spr) or "0") + "};"]) + "\n")
    sc.im.resize((W * 3, H * 3), Image.NEAREST).save(out.with_suffix(".png"))
    return len(tiles)


C = {k: rgb(v) for k, v in dict(
    sky_hi="#4878C8", sky="#68A0E0", sky_lo="#A8D8F0", white="#FFFFFF", cloud_lo="#C0D8F0",
    far="#78A8A8", hill="#58A058", grass="#70B048", grass_d="#3C7838", path="#D8C090", path_d="#B09058",
    st_l="#E0DCD8", st_m="#A8A8B8", st_d="#686880", deep="#383850",
    roof_l="#F06050", roof="#C83838", roof_d="#801C30",
    wd_l="#D8A060", wd="#A86838", wd_d="#6C3C20",
    au_l="#FFF0A0", au="#F0C040", au_d="#A07020",
    water="#3870C0", water_l="#80B8E8",
    navy="#182040", cream="#F8E8C0", ink="#202030", blue="#3858B0", blue_d="#202C70",
    flame="#FFA028", flame_l="#FFE878", skin="#F0C090", skin_d="#C89060",
    wall="#8088A0", wall_l="#9AA2B8", wall_d="#606880", wall_w="#A09078", wall_wl="#B8AC90", wall_wd="#787060",
).items()}


def auto_pals(im, limit=8):
    """Group the colour sets of all 8x8 cells into at most `limit` palettes of
    four. Greedy: largest sets first, each joins the palette that absorbs it
    with the least growth. Returns (palettes, cells_over_four)."""
    sets, over = {}, []
    for cy in range(H // 8):
        for cx in range(W // 8):
            cols = frozenset(im.getpixel((cx * 8 + x, cy * 8 + y)) for y in range(8) for x in range(8))
            if len(cols) > 4:
                over.append((cx, cy, sorted(cols)))
            else:
                sets[cols] = sets.get(cols, 0) + 1
    pals = []
    for cols in sorted(sets, key=lambda c: (-len(c), -sets[c])):
        best, grow = None, 9
        for p in pals:
            u = p | cols
            if len(u) <= 4 and len(u) - len(p) < grow:
                best, grow = p, len(u) - len(p)
        if best is None:
            pals.append(set(cols))
        else:
            best |= cols
    merged = True
    while merged and len(pals) > limit:
        merged = False
        for i in range(len(pals)):
            for j in range(i + 1, len(pals)):
                if len(pals[i] | pals[j]) <= 4:
                    pals[i] |= pals[j]
                    del pals[j]
                    merged = True
                    break
            if merged:
                break
    out = []
    for p in pals:
        p = sorted(p, key=lambda c: -sum(c))
        while len(p) < 4:
            p.append((0, 0, 0) if (0, 0, 0) not in p else (8, 8, 8))
        out.append(p)
    return out, over


def finish(sc, sprites=(), pals=None):
    if pals is not None:
        bad = []
        for cy in range(H // 8):
            for cx in range(W // 8):
                cols = {sc.im.getpixel((cx * 8 + x, cy * 8 + y)) for y in range(8) for x in range(8)}
                if not any(cols <= set(p) for p in pals):
                    bad.append((cx, cy, sorted(cols)))
        for cx, cy, cols in bad[:12]:
            print("  no palette for cell", (cx, cy), ["#%02X%02X%02X" % c for c in cols], file=sys.stderr)
        if bad:
            raise SystemExit(f"{len(bad)} cells fit no palette")
        return sc, pals, list(sprites)
    pals, over = auto_pals(sc.im)
    for cx, cy, cols in over[:12]:
        print("  >4 colours at cell", (cx, cy), ["#%02X%02X%02X" % c for c in cols], file=sys.stderr)
    if over:
        raise SystemExit(f"{len(over)} cells over four colours")
    if len(pals) > 8:
        for p in pals:
            print("  palette", ["#%02X%02X%02X" % c for c in p], file=sys.stderr)
        raise SystemExit(f"{len(pals)} palettes needed")
    return sc, pals, list(sprites)


def put(sc, x, y, c):
    if 0 <= x < W and 0 <= y < H:
        sc.im.putpixel((x, y), c)


def hline(sc, x0, x1, y, c):
    sc.d.line([(x0, y), (x1, y)], fill=c)


def vline(sc, x, y0, y1, c):
    sc.d.line([(x, y0), (x, y1)], fill=c)


def dither_row(sc, x0, x1, y, a, b, phase=0):
    for x in range(x0, x1 + 1):
        put(sc, x, y, a if (x + y + phase) % 2 else b)


def bevel_text(sc, x, y, s, f, body, rim, outline):
    """Outlined letters with a lit rim on the upper-left edges."""
    for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (1, -1), (-1, 1), (1, 1)):
        sc.text(x + dx, y + dy, s, f, outline)
    sc.text(x - 1, y - 1, s, f, rim)
    sc.text(x, y, s, f, body)


CROWN5 = ["#..#..#", "#.#.#.#", "#######", ".#####.", ".#####."]


def wordmark(sc, x, y, s, f, body, shade, outline):
    """Letters from a mask: 1px outline, a 1px shade on the lower inside
    edges, and the crown of the i replacing its dot."""
    from PIL import Image, ImageDraw
    pad = 8
    w = int(f.getlength(s)) + pad * 2
    m = Image.new("1", (w, 40), 0)
    d = ImageDraw.Draw(m)
    d.fontmode = "1"
    d.text((pad, pad), s, font=f, fill=1)
    px = m.load()
    i0, i1 = pad + int(f.getlength(s[:s.index("i")])), pad + int(f.getlength(s[:s.index("i") + 1]))
    rows = [yy for yy in range(40) if any(px[xx, yy] for xx in range(i0, i1))]
    top = rows[0]
    gap = next(yy for yy in range(top, 40) if not any(px[xx, yy] for xx in range(i0, i1)))
    for yy in range(top, gap):  # the dot goes
        for xx in range(i0, i1):
            px[xx, yy] = 0
    stem = [xx for xx in range(i0, i1) if px[xx, gap + 1]]
    cx = (stem[0] + stem[-1]) // 2
    for r, row in enumerate(CROWN5):  # the crown sits where the dot was
        for c, ch in enumerate(row):
            if ch == "#":
                px[cx - 3 + c, gap - len(CROWN5) + r] = 1
    ink = {(xx, yy) for yy in range(40) for xx in range(w) if px[xx, yy]}
    # the font draws each stroke as a thin far edge, a gap and the body: close
    # the gap, then paint the two extruded columns and the top row darker
    closed = set(ink)
    for xx in range(1, w - 1):
        for yy in range(1, 39):
            # a gap whose left neighbour is a thin line belongs to the glyph;
            # the gap between two letters has a body to its left
            if (xx, yy) not in ink and (xx - 1, yy) in ink and (xx - 2, yy) not in ink and (xx + 1, yy) in ink:
                closed.add((xx, yy))
    for xx, yy in closed:
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                if (xx + dx, yy + dy) not in closed:
                    put(sc, x + xx + dx - pad, y + yy + dy - pad, outline)
    for xx, yy in closed:
        side = (xx - 1, yy) not in closed or (xx - 2, yy) not in closed or (xx, yy - 1) not in closed
        put(sc, x + xx - pad, y + yy - pad, shade if side else body)
    return w - pad * 2


def small_cloud(sc, x, y):
    """A 7-row cloud for one scroll band."""
    for dx, dy, r in ((3, 4, 3), (8, 2, 4), (14, 4, 3)):
        sc.d.ellipse((x + dx - r, y + dy - r, x + dx + r, y + dy + r), fill=C["white"])
    sc.d.rectangle((x, y + 4, x + 17, y + 6), fill=C["white"])


def shadow_text(sc, x, y, s, f, body, shadow):
    sc.text(x + 1, y + 1, s, f, shadow)
    sc.text(x, y, s, f, body)


def stonework(sc, x0, y0, x1, y1, fill, light, dark, seed=1, course=6):
    """Irregular ashlar: courses of varied length, mortar in the dark tone,
    a few blocks lightened, the odd one darkened."""
    rnd = random.Random(seed)
    sc.d.rectangle((x0, y0, x1, y1), fill=fill)
    y = y0
    row = 0
    while y <= y1:
        h = course
        x = x0 + (rnd.choice((0, 4, 8)) if row else 0) - 8
        while x <= x1:
            w = rnd.choice((12, 14, 16, 20))
            bx0, bx1 = max(x, x0), min(x + w - 1, x1)
            by1 = min(y + h - 1, y1)
            if bx1 >= bx0:
                r = rnd.random()
                if r < 0.12:
                    sc.d.rectangle((bx0, y, bx1, by1), fill=light)
                elif r < 0.2:
                    sc.d.rectangle((bx0, y, bx1, by1), fill=dark)
                if x + w - 1 <= x1:
                    vline(sc, x + w - 1, y, by1, dark)
            x += w
        hline(sc, x0, x1, min(y + h - 1, y1), dark)
        y += h
        row += 1


def masonry(sc, x0, y0, x1, y1, tone, seed=1, course=6):
    """Ashlar courses whose fill and mortar tones come from tone(x), so the
    joints run on through lit and shadowed faces."""
    rnd = random.Random(seed)
    for x in range(x0, x1 + 1):
        vline(sc, x, y0, y1, tone(x)[0])
    y, row = y0, 0
    while y <= y1:
        x = x0 + (rnd.choice((0, 4, 8)) if row else 0) - 8
        by1 = min(y + course - 1, y1)
        while x <= x1:
            w = rnd.choice((10, 12, 14, 16))
            jx = x + w - 1
            if x0 <= jx <= x1:
                vline(sc, jx, y, by1, tone(jx)[1])
            x += w
        for px in range(x0, x1 + 1):
            put(sc, px, by1, tone(px)[1])
        y += course
        row += 1


def tower(sc, x0, w, y0, y1):
    """A round tower: lit, mid and shadow bands with the courses running through."""
    a, b = x0 + w * 3 // 8, x0 + w * 5 // 8

    def tone(x):
        if x < a:
            return C["st_l"], C["st_m"]
        if x < b:
            return C["st_m"], C["st_d"]
        return C["st_d"], C["deep"]
    masonry(sc, x0, y0, x0 + w - 1, y1, tone, seed=x0, course=6)
    vline(sc, x0, y0, y1, C["st_m"])
    vline(sc, x0 + w - 1, y0, y1, C["st_d"])


def cone_roof(sc, cx, y_top, y_base, half):
    """A cone lit from the left: three bands by x, tile lines on the shadow side."""
    for y in range(y_top, y_base + 1):
        t = (y - y_top) / (y_base - y_top)
        hw = max(1, round(half * t))
        for x in range(cx - hw, cx + hw + 1):
            u = (x - cx) / max(hw, 1)
            put(sc, x, y, C["roof_l"] if u < -0.3 else C["roof"] if u < 0.4 else C["roof_d"])
        if y > y_top + 4 and (y - y_top) % 3 == 0:
            hline(sc, cx + 1, cx + hw - 1, y, C["roof_d"])
        put(sc, cx - hw, y, C["roof"])
        put(sc, cx + hw, y, C["roof_d"])
    hline(sc, cx - half - 1, cx + half + 1, y_base, C["roof_d"])
    hline(sc, cx - half - 1, cx + half + 1, y_base + 1, C["roof_d"])


def pennant(sc, x, y_top, h):
    vline(sc, x, y_top, y_top + h, C["roof_d"])
    for i in range(6):
        hline(sc, x + 1, x + 1 + 7 - i, y_top + i, C["roof"] if i < 3 else C["roof_d"])


def battlements(sc, x0, x1, y_top):
    for x in range(x0, x1, 8):
        sc.d.rectangle((x, y_top, x + 3, y_top + 3), fill=C["st_l"])
        vline(sc, x + 3, y_top, y_top + 3, C["st_d"])
    hline(sc, x0, x1 - 1, y_top + 4, C["st_d"])  # shadow under the crenels


def arch_window(sc, x, y, w, h, fill):
    sc.d.rectangle((x, y + w // 2, x + w - 1, y + h - 1), fill=fill)
    sc.d.ellipse((x, y, x + w - 1, y + w - 1), fill=fill)
    hline(sc, x - 1, x + w, y + h, C["st_l"])  # sill


def cloud(sc, x, y):
    for dx, dy, r in ((0, 4, 4), (6, 1, 5), (13, 3, 4), (7, 6, 4)):
        sc.d.ellipse((x + dx - r, y + dy - r, x + dx + r, y + dy + r), fill=C["white"])
    sc.d.rectangle((x - 4, y + 5, x + 17, y + 8), fill=C["white"])


def bush(sc, cx, cy, r):
    for dx, dy, rr in ((0, 0, r), (-r, r // 3, r * 2 // 3), (r, r // 3, r * 2 // 3)):
        sc.d.ellipse((cx + dx - rr, cy + dy - rr, cx + dx + rr, cy + dy + rr), fill=C["grass_d"])
    for dx, dy, rr in ((-r // 3, -r // 3, r // 2), (r // 2, -r // 4, r // 3)):
        sc.d.ellipse((cx + dx - rr, cy + dy - rr, cx + dx + rr, cy + dy + rr), fill=C["grass"])


CROWN = ["#..#..#.", "#.###.#.", "#######.", ".#####..", ".#####..", "#######.", "........", "........"]
CROWN_L = ["........", "........", "........", ".#.#.#..", ".#......", "........", "........", "........"]


def crown(sc, x, y):
    sc.rows(x, y, CROWN, {"#": C["au"]})
    sc.rows(x, y, ["........", "........", "........", "........", "........", "#######.", "........", "........"], {"#": C["au_d"]})
    sc.rows(x, y, CROWN_L, {"#": C["au_l"]})


# ---- title -----------------------------------------------------------------

def title():
    sc = Scene(C["sky_lo"])
    # two cloud bands, rows 5 and 6, each scrolled on its own in the ROM
    small_cloud(sc, 20, 40)
    small_cloud(sc, 110, 40)
    small_cloud(sc, 60, 48)
    small_cloud(sc, 136, 48)
    # distant hills, then nearer ones
    for cx, r in ((-20, 60), (175, 56)):
        sc.d.ellipse((cx - r, 88, cx + r, 88 + r), fill=C["far"])
    for cx, r in ((-6, 36), (166, 40)):
        sc.d.ellipse((cx - r, 104, cx + r, 104 + r), fill=C["hill"])
    # curtain wall
    masonry(sc, 40, 88, 119, 119, lambda x: (C["st_l"], C["st_m"]), seed=3, course=8)
    hline(sc, 40, 119, 88, C["st_m"])
    for x in list(range(40, 44)) + list(range(104, 108)):  # shadows cast on the wall
        for y in range(88, 120):
            c = sc.im.getpixel((x, y))
            put(sc, x, y, C["st_m"] if c == C["st_l"] else C["st_d"] if c == C["st_m"] else c)
    battlements(sc, 40, 120, 84)
    # the gatehouse: lit face, shadow face, crest, arch and portcullis
    masonry(sc, 56, 64, 103, 119, lambda x: (C["st_l"], C["st_m"]) if x < 96 else (C["st_d"], C["deep"]), seed=5, course=8)
    sc.d.rectangle((56, 61, 95, 63), fill=C["st_l"])
    sc.d.rectangle((96, 61, 103, 63), fill=C["st_d"])
    vline(sc, 103, 61, 119, C["st_d"])
    vline(sc, 56, 61, 119, C["st_m"])
    battlements(sc, 56, 104, 56)
    shield(sc, 68, 64)
    arch_window(sc, 70, 88, 20, 32, C["st_d"])  # the arch ring
    arch_window(sc, 72, 90, 16, 30, C["deep"])
    sc.d.rectangle((78, 88, 81, 89), fill=C["st_m"])  # keystone
    for x in (75, 79, 83):  # portcullis, bars from the arch down
        y = 90
        while sc.im.getpixel((x, y)) != C["deep"]:
            y += 1
        vline(sc, x, y + 1, 119, C["st_d"])
    for y in range(99, 120, 5):
        hline(sc, 73, 86, y, C["st_d"])
    # towers with cone roofs
    for x0 in (16, 120):
        tower(sc, x0, 24, 72, 119)
        hline(sc, x0 + 1, x0 + 22, 72, C["st_d"])  # eave shadow
        hline(sc, x0 + 1, x0 + 22, 73, C["st_d"])
        arch_window(sc, x0 + 7, 82, 4, 8, C["deep"])
        arch_window(sc, x0 + 7, 104, 4, 8, C["deep"])
        cone_roof(sc, x0 + 12, 56, 70, 13)
    # moat and drawbridge
    sc.d.rectangle((0, 120, W, 127), fill=C["water"])
    for x in range(0, W, 12):
        hline(sc, x + (x // 12 % 2) * 3, x + 4 + (x // 12 % 2) * 3, 122 + (x // 12 % 3), C["water_l"])
    sc.d.rectangle((72, 120, 87, 127), fill=C["wd"])
    for x in range(72, 88, 4):
        vline(sc, x, 120, 127, C["wd_d"])
    for x, x2 in ((70, 72), (89, 87)):
        for y in range(108, 119, 2):
            put(sc, x + (x2 - x) * (y - 108) // 12, y, C["deep"])
    # grass bank, path, bushes
    sc.d.rectangle((0, 128, W, H), fill=C["grass"])
    hline(sc, 0, W, 128, C["grass_d"])
    sc.d.polygon([(70, 129), (89, 129), (96, 143), (63, 143)], fill=C["path"])
    bush(sc, 10, 138, 7)
    bush(sc, 150, 139, 7)
    bush(sc, 46, 141, 5)
    bush(sc, 116, 141, 5)
    # wordmark and subtitle (shown through the window layer in the ROM)
    w = int(LOGO19.getlength("Kingside"))
    wordmark(sc, (W - w) // 2, 3, "Kingside", LOGO19, C["au"], C["au_d"], C["ink"])
    sub = "Chess Academy"
    w, h = sc.ink_size(sub, SMALL)
    x = (W - w) // 2
    for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
        sc.text(x + dx, 28 + dy, sub, SMALL, C["ink"])
    sc.text(x, 28, sub, SMALL, C["au"])
    shadow_text(sc, (W - sc.ink_size("Push Start", SMALL)[0]) // 2, 133, "Push Start", SMALL, C["white"], C["grass_d"])
    pals = [
        [C["sky_lo"], C["au"], C["au_d"], C["ink"]],
        [C["sky_lo"], C["white"], C["far"], C["hill"]],
        [C["sky_lo"], C["roof_l"], C["roof"], C["roof_d"]],
        [C["st_l"], C["st_m"], C["st_d"], C["sky_lo"]],
        [C["st_l"], C["st_m"], C["st_d"], C["deep"]],
        [C["water"], C["water_l"], C["wd"], C["wd_d"]],
        [C["grass"], C["grass_d"], C["path"], C["white"]],
        [C["st_l"], C["st_m"], C["blue"], C["au"]],
    ]
    return finish(sc, pals=pals)


# ---- menu ------------------------------------------------------------------

MODES = ["Campaign", "Coaching", "Puzzles", "VS Computer", "2 Players"]


def torch(sc, x, y):
    """Flame in the cell at (x, y), a wooden handle in an iron ring below."""
    for row, (l, r) in enumerate(((3, 4), (2, 5), (1, 6), (0, 7), (0, 7), (1, 6), (1, 6), (2, 5))):
        hline(sc, x + l, x + r, y + row, C["flame"])
    for row, (l, r) in enumerate(((3, 4), (3, 4), (2, 5), (2, 5), (3, 4)), start=3):
        hline(sc, x + l, x + r, y + row, C["flame_l"])
    sc.d.rectangle((x + 3, y + 8, x + 4, y + 15), fill=C["wd"])
    vline(sc, x + 5, y + 8, y + 15, C["wd_d"])
    vline(sc, x + 2, y + 8, y + 15, C["wd_d"])
    sc.d.rectangle((x + 1, y + 10, x + 6, y + 11), fill=C["deep"])
    sc.d.rectangle((x + 1, y + 11, x + 1, y + 15), fill=C["deep"])
    sc.d.rectangle((x + 6, y + 11, x + 6, y + 15), fill=C["deep"])


def plank(sc, x0, y0, x1, y1):
    sc.d.rectangle((x0, y0, x1, y1), fill=C["wd"])
    for y in range(y0 + 3, y1 - 2, 4):  # grain
        for x in range(x0 + 6 + (y % 8), x1 - 6, 23):
            hline(sc, x, x + 9, y, C["wd_d"])
    sc.d.rectangle((x0, y0, x1, y1), outline=C["wd_d"])
    hline(sc, x0 + 1, x1 - 1, y0 + 1, C["wd_l"])
    vline(sc, x0 + 1, y0 + 1, y1 - 1, C["wd_l"])
    for x in (x0 + 3, x1 - 3):  # iron straps
        vline(sc, x, y0 + 1, y1 - 1, C["deep"])
        vline(sc, x + 1, y0 + 1, y1 - 1, C["deep"])


def chain(sc, x, y0, y1):
    for y in range(y0, y1, 3):
        put(sc, x, y, C["deep"])
        put(sc, x, y + 1, C["deep"])
        put(sc, x, y + 2, C["deep"])


def shield(sc, x0, y0):
    """A 24x24 crest: gold rim, blue field, the white rook from the pack."""
    pts = [(x0, y0), (x0 + 23, y0), (x0 + 23, y0 + 14), (x0 + 12, y0 + 23), (x0 + 11, y0 + 23), (x0, y0 + 14)]
    sc.d.polygon(pts, fill=C["au"])
    inner = [(x0 + 2, y0 + 2), (x0 + 21, y0 + 2), (x0 + 21, y0 + 13), (x0 + 12, y0 + 20), (x0 + 11, y0 + 20), (x0 + 2, y0 + 13)]
    sc.d.polygon(inner, fill=C["blue"])
    sc.rows(x0 + 8, y0 + 6, CROWN, {"#": C["au"]})


def menu():
    sc = Scene(C["wall"])
    stonework(sc, 0, 0, W - 1, H - 1, C["wall"], C["wall_l"], C["wall_d"], seed=11, course=8)
    sel = 0
    # warm torchlight around the chosen plank (a palette swap per cell in the game)
    warm = []
    for tx in (1,):
        ty = 3 + 3 * sel
        for cy in range(0, 18):
            for cx in range(0, 20):
                if abs(cx - tx) * 2 + abs(cy - ty - 1) <= 5 and (cx, cy) not in warm:
                    warm.append((cx, cy))
    for cx, cy in warm:
        box = (cx * 8, cy * 8, cx * 8 + 8, cy * 8 + 8)
        region = sc.im.crop(box)
        px = region.load()
        for y in range(region.height):
            for x in range(region.width):
                px[x, y] = {C["wall"]: C["wall_w"], C["wall_l"]: C["wall_wl"], C["wall_d"]: C["wall_wd"]}[px[x, y]]
        sc.im.paste(region, box[:2])
    sc.d.rectangle((64, 0, 95, 23), fill=C["wall"])
    shield(sc, 68, 0)
    for i, label in enumerate(MODES):
        y0 = 24 + 24 * i
        if i:
            chain(sc, 34, y0 - 8, y0)
            chain(sc, 125, y0 - 8, y0)
        plank(sc, 24, y0, 135, y0 + 15)
        sc.fits(label, MENU, 40, 127)
        w, h = sc.ink_size(label, MENU)
        shadow_text(sc, 40, y0 + 8 - h // 2, label, MENU, C["cream"], C["wd_d"])
    # cast shadows under the planks
    for i in range(len(MODES)):
        y = 24 + 24 * i + 16
        for x in range(26, 136):
            if sc.im.getpixel((x, y)) in (C["wall"], C["wall_l"]):
                put(sc, x, y, C["wall_d"])
    sc.d.rectangle((0, 144, W, H), fill=C["roof"])
    for x0 in (8,):  # the cursor: a torch beside the chosen plank (a sprite in the game)
        sc.d.rectangle((x0, 24 + 24 * sel, x0 + 7, 24 + 24 * sel + 15), fill=C["wall_w"])
        torch(sc, x0, 24 + 24 * sel)
    return finish(sc)


# ---- lesson ----------------------------------------------------------------
# N bg, D deep, M mid steel, L light steel, S skin, K skin shadow, G gold.
KNIGHT = [
    "NNNNNNNNNNNNNNNGGNNNNNNNNNNNNNNN",
    "NNNNNNNNNNNNNNGGGGNNNNNNNNNNNNNN",
    "NNNNNNNNNNNNNNGGGGNNNNNNNNNNNNNN",
    "NNNNNNNNNNNNNNNGGNNNNNNNNNNNNNNN",
    "NNNNNNNNNNNDDLLLLLLDDNNNNNNNNNNN",
    "NNNNNNNNNDDLLLLLLLLLLDDNNNNNNNNN",
    "NNNNNNNNDLLLLLLLLMMMMMMDNNNNNNNN",
    "NNNNNNNDLLLLLLLMMMMMMMMMDNNNNNNN",
    "NNNNNNDLLLLLLMMMMMMMMMMMMDNNNNNN",
    "NNNNNDLLLLLMMMMMMMMMMMMMMMDNNNNN",
    "NNNNNDLLLLMMMMMMMMMMMMMMMMDNNNNN",
    "NNNNDLLLLMMMMMMMMMMMMMMMMMMDNNNN",
    "NNNNDDDDDDDDDDDDDDDDDDDDDDDDNNNN",
    "NNNDDSSSSSSSSSSSSSSSSSSSSSSDDNNN",
    "NNNNDDDDDDDDDDDDDDDDDDDDDDDDNNNN",
    "NNNNNDDDDDDDDDDDDDDDDDDDDDDNNNNN",
    "NNNNNDSSSSSSSSSSSSSSSSSSSDNNNNNN",
    "NNNNNDSSSDDDSSSSSSSDDDSSSDNNNNNN",
    "NNNNNDSSSSDDSSSSSSSDDSSSSDNNNNNN",
    "NNNNNDSSSSSSSSSSSSSSSSSSSDNNNNNN",
    "NNNNNDSSSSSSSSDDSSSSSSSSSDNNNNNN",
    "NNNNNDSSSSSSSSSDSSSSSSSSSDNNNNNN",
    "NNNNNDSSSSSSSSDDDSSSSSSSSDNNNNNN",
    "NNNNNDLLLLSSSSSSSSSSSLLLLDNNNNNN",
    "NNNNNDLLLLLLSSDDDDSSLLLLLDNNNNNN",
    "NNNNNDLLLLLLLLLLLLLLLLLLLDNNNNNN",
    "NNNNNNDLLLDLLLLLLLLLDLLLDNNNNNNN",
    "NNNNNNDLLLLDLLLLLLLDLLLLDNNNNNNN",
    "NNNNDDDDLLLLLLLLLLLLLLLDDDDNNNNN",
    "NNNDMMMMDDLLLLLLLLLLLDDMMMMDNNNN",
    "NNNDMMMMMMMDDLLLLLDDMMMMMMMDNNNN",
    "NNNDMMMMMMMMMDDDDDMMMMMMMMMDNNNN",
]
KMAP = {"D": C["deep"], "M": C["st_m"], "L": C["st_l"], "S": C["skin"], "K": C["skin_d"], "G": C["au"]}


def gold_box(sc, x0, y0, x1, y1, fill):
    sc.d.rectangle((x0, y0, x1, y1), fill=fill)
    sc.d.rectangle((x0, y0, x1, y1), outline=C["au_d"])
    sc.d.rectangle((x0 + 1, y0 + 1, x1 - 1, y1 - 1), outline=C["au"])
    sc.d.rectangle((x0 + 2, y0 + 2, x1 - 2, y1 - 2), outline=C["au_d"])
    for cx, cy in ((x0 + 1, y0 + 1), (x1 - 1, y0 + 1), (x0 + 1, y1 - 1), (x1 - 1, y1 - 1)):
        sc.d.rectangle((cx - 1, cy - 1, cx + 1, cy + 1), fill=C["au"])
        put(sc, cx, cy, C["au_d"])


def banner(sc, x0, y0, w, h):
    sc.d.rectangle((x0, y0, x0 + w - 1, y0 + h - 1), fill=C["blue"])
    sc.d.polygon([(x0, y0 + h), (x0 + w - 1, y0 + h), (x0 + w // 2, y0 + h + 9)], fill=C["blue"])
    sc.d.rectangle((x0 + w - 6, y0, x0 + w - 1, y0 + h - 1), fill=C["blue_d"])  # a fold in shadow
    sc.d.polygon([(x0 + w // 2, y0 + h), (x0 + w - 1, y0 + h), (x0 + w // 2, y0 + h + 9)], fill=C["blue_d"])
    sc.d.rectangle((x0, y0, x0 + w - 1, y0 + h - 1), outline=C["au"])
    sc.d.line([(x0, y0 + h - 1), (x0 + w // 2, y0 + h + 8), (x0 + w - 1, y0 + h - 1)], fill=C["au"])



def lesson():
    sc = Scene(C["wall"])
    stonework(sc, 0, 0, W - 1, 87, C["wall"], C["wall_l"], C["wall_d"], seed=7, course=8)
    # a window onto the banded sky, its frame inside the window's own cells
    sc.d.rectangle((104, 8, 135, 55), fill=C["wall_l"])
    sc.d.rectangle((106, 10, 133, 23), fill=C["sky"])
    sc.d.rectangle((106, 24, 133, 45), fill=C["sky_lo"])
    sc.d.rectangle((106, 46, 133, 53), fill=C["hill"])
    vline(sc, 119, 10, 53, C["wall_l"])
    vline(sc, 120, 10, 53, C["wall_l"])
    hline(sc, 106, 133, 24, C["wall_l"])
    hline(sc, 102, 137, 56, C["wall_d"])
    sc.d.rectangle((56, 40, 87, 55), fill=C["wall"])
    banner(sc, 56, 0, 32, 46)
    sc.rows(64, 14, PIECES["P"], {"o": C["white"], "h": C["white"], "u": C["blue_d"], "r": C["blue"]})
    # the mentor
    gold_box(sc, 8, 24, 47, 71, C["navy"])
    for y, r in enumerate(KNIGHT):
        for x, ch in enumerate(r):
            if ch != "N":
                put(sc, 12 + x, 28 + y, KMAP[ch])
    gold_box(sc, 8, 72, 71, 87, C["navy"])
    sc.text_center(8, 71, 79, "Sir Rook", SMALL, C["au"])
    gold_box(sc, 0, 88, W - 1, H - 1, C["navy"])
    lines = ["A knight can hit two", "pieces at once.", "That is a fork!"]
    for i, line in enumerate(lines):
        sc.fits(line, SMALL, 10, W - 11)
        sc.text(10, 97 + 16 * i, line, SMALL, C["white"])
    sc.rows(W - 16, H - 11, ["#####", ".###.", "..#.."], {"#": C["au"]})
    pals = [
        [C["wall"], C["wall_l"], C["wall_d"], C["deep"]],
        [C["wall"], C["blue"], C["blue_d"], C["au"]],
        [C["blue"], C["blue_d"], C["au"], C["white"]],
        [C["sky"], C["sky_lo"], C["hill"], C["wall_l"]],
        [C["navy"], C["au"], C["au_d"], C["white"]],
        [C["navy"], C["deep"], C["st_l"], C["st_m"]],
        [C["navy"], C["deep"], C["skin"], C["st_l"]],
        [C["navy"], C["au"], C["au_d"], C["deep"]],
    ]
    return finish(sc, pals=pals)


# ---- board -----------------------------------------------------------------

def board(fen="rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w", marked=("e7", "e5"), cursor="g1", dots=("f3", "h3")):
    light, dark = C["cream"], C["blue"]
    fill, ink = C["white"], C["ink"]
    hi_l, hi_d = rgb("#F8E070"), rgb("#D8A840")
    sc = Scene(C["wall"])
    stonework(sc, 0, 0, W - 1, H - 1, C["wall"], C["wall_l"], C["wall_d"], seed=9, course=8)
    sc.d.rectangle((0, 8, 15, H), fill=C["wall"])
    sc.d.rectangle((0, 136, W, H), fill=C["wall"])
    sc.d.rectangle((144, 8, W, H), fill=C["wall"])
    sc.d.rectangle((0, 0, W, 7), fill=C["navy"])
    hline(sc, 0, W, 7, C["au"])
    crown(sc, 3, 0)
    sc.text(16, 1, "Your move", SMALL, C["white"])

    def ring_of(sq, other):
        return mix(sq, other, 0.25 if sq in (light, hi_l) else 0.45)

    place, r, c = {}, 7, 0
    for ch in fen.split()[0]:
        if ch == "/":
            r, c = r - 1, 0
        elif ch.isdigit():
            c += int(ch)
        else:
            place[(c, r)] = ch
            c += 1
    for rank in range(8):
        for f in range(8):
            sq = "abcdefgh"[f] + str(rank + 1)
            x0, y0 = 16 + 16 * f, 8 + 16 * (7 - rank)
            is_light = (f + rank) % 2 == 1
            base = (hi_l if is_light else hi_d) if sq in marked else (light if is_light else dark)
            sc.d.rectangle((x0, y0, x0 + 15, y0 + 15), fill=base)
            if (f, rank) in place:
                rr = ring_of(base, dark if is_light else light)
                sc.rows(x0, y0, PIECES[place[(f, rank)]], {"r": rr, "h": fill, "o": fill, "u": ink})
            if sq in marked:
                for ax, ay, sx, sy in ((0, 0, 1, 1), (15, 0, -1, 1), (0, 15, 1, -1), (15, 15, -1, -1)):
                    for i in range(3):
                        put(sc, x0 + ax + sx * i, y0 + ay, ink)
                        put(sc, x0 + ax, y0 + ay + sy * i, ink)
    # the board is set into the stone: shadow above and left, light below and right
    vline(sc, 15, 8, 135, C["deep"])
    vline(sc, 144, 8, 136, C["wall_l"])
    hline(sc, 15, 144, 136, C["wall_l"])
    lab = {"u": C["deep"], "p": C["cream"], "o": C["deep"]}
    for rank in range(8):
        sc.rows(7, 8 + 16 * (7 - rank) + 4, DIGITS[rank], lab)
    for f in range(8):
        sc.rows(16 + 16 * f + 4, 137, LETTERS[f], lab)
    sq_pal = lambda b, o: [b, ring_of(b, o), fill, ink]
    sprites = [("cursor", cursor)] + [("dot", d) for d in dots]
    return finish(sc, sprites)


SCREENS = {"title": title, "menu": menu, "lesson": lesson, "board": board}

if __name__ == "__main__":
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else GAME / "build" / "art"
    out.mkdir(parents=True, exist_ok=True)
    failed = False
    for name, fn in SCREENS.items():
        try:
            print(name, compile_scene(*fn(), out / f"castle_{name}.c"), "tiles")
        except SystemExit as e:
            print(e, file=sys.stderr)
            failed = True
    sys.exit(1 if failed else 0)
