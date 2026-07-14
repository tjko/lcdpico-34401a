#!/usr/bin/python3
#

import sys
import re, math
from pathlib import Path


OUT = "./outputs"

out_path = Path(OUT)
if not out_path.is_dir():
    sys.exit(f"Cannot find directory: {OUT}")


W, H = 410, 300          # half previous width (820 -> 410), same height
CX, CY = W/2, H/2

DEFS = '''  <defs>
    <linearGradient id="digit" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#ffeec2"/>
      <stop offset="45%" stop-color="#ffb449"/>
      <stop offset="100%" stop-color="#ff860f"/>
    </linearGradient>
    <filter id="hugeGlow" x="-80%" y="-80%" width="260%" height="260%">
      <feGaussianBlur stdDeviation="18"/>
    </filter>
    <filter id="bigGlow" x="-60%" y="-60%" width="220%" height="220%">
      <feGaussianBlur stdDeviation="10"/>
    </filter>
    <filter id="midGlow" x="-40%" y="-40%" width="180%" height="180%">
      <feGaussianBlur stdDeviation="3.2"/>
    </filter>
__EXTRA_DEFS__  </defs>
'''

HEAD = f'<svg viewBox="0 0 {W} {H}" xmlns="http://www.w3.org/2000/svg" font-family="Arial, Helvetica, sans-serif">\n'
BG = f'  <rect x="0" y="0" width="{W}" height="{H}" fill="#000000"/>\n'
TAIL = '</svg>\n'

# Rounded-rectangle frame, fixed size, centered in the 410x300 canvas
FRAME = '''  <rect x="30" y="50" width="350" height="200" rx="32" ry="32" fill="none"
        stroke="#ff7e1e" stroke-width="7" opacity="0.6" filter="url(#bigGlow)"/>
  <rect x="30" y="50" width="350" height="200" rx="32" ry="32" fill="none"
        stroke="#ffb84e" stroke-width="3"/>
  <rect x="30" y="50" width="350" height="200" rx="32" ry="32" fill="none"
        stroke="#fff1cf" stroke-width="1" opacity="0.65"/>
'''

# ---------- Text image (4W) ----------
def build_text(word, fs=170, framed=False, yoffset=0):
    body = ""
    layers = [
        ('#ff7414', '0.6', 'hugeGlow'),
        ('#ff7e1e', '0.88', 'bigGlow'),
        ('#ffa838', '0.95', 'midGlow'),
    ]
    for fill, op, filt in layers:
        body += (f'  <text x="{CX}" y="{CY+2+yoffset}" dominant-baseline="central" text-anchor="middle" '
                 f'font-size="{fs}" fill="{fill}" opacity="{op}" filter="url(#{filt})">{word}</text>\n')
    body += (f'  <text x="{CX}" y="{CY+2+yoffset}" dominant-baseline="central" text-anchor="middle" '
             f'font-size="{fs}" fill="url(#digit)" stroke="#fff1cf" stroke-width="1.4">{word}</text>\n')
    frame = FRAME if framed else ""
    return HEAD + DEFS.replace("__EXTRA_DEFS__", "") + BG + frame + body + TAIL

# ---------- Symbol glow layering via <use> + currentColor ----------
def symbol_uses(sym_id):
    layers = [
        ('#ff7414', '0.6', 'hugeGlow'),
        ('#ff7e1e', '0.88', 'bigGlow'),
        ('#ffa838', '0.95', 'midGlow'),
        ('#ffd27a', '1', None),   # bright crisp core
    ]
    out = ""
    for color, op, filt in layers:
        f = f' filter="url(#{filt})"' if filt else ""
        out += f'  <use href="#{sym_id}" color="{color}" opacity="{op}"{f}/>\n'
    return out

# ---------- Diode schematic symbol ----------
def diode_def():
    return ('''    <g id="diodeSym" stroke-width="10" stroke-linecap="round" stroke-linejoin="round">
      <line x1="70" y1="150" x2="160" y2="150" stroke="currentColor" fill="none"/>
      <polygon points="160,100 160,200 220,150" fill="currentColor" stroke="currentColor"/>
      <line x1="220" y1="100" x2="220" y2="200" stroke="currentColor" fill="none"/>
      <line x1="220" y1="150" x2="340" y2="150" stroke="currentColor" fill="none"/>
    </g>
''')

def build_diode(framed=False):
    defs = DEFS.replace("__EXTRA_DEFS__", diode_def())
    frame = FRAME if framed else ""
    return HEAD + defs + BG + frame + symbol_uses("diodeSym") + TAIL

# ---------- Continuity (sound-wave) multimeter symbol ----------
def continuity_def():
    cx, cy = 150, 150
    radii = [50, 90, 130]
    ang = math.radians(38)
    arcs = ""
    for r in radii:
        sx = cx + r*math.cos(-ang); sy = cy + r*math.sin(-ang) * 0.8
        ex = cx + r*math.cos(ang);  ey = cy + r*math.sin(ang) * 0.8
        arcs += (f'      <path d="M {sx:.1f} {sy:.1f} A {r} {r} 0 0 1 {ex:.1f} {ey:.1f}" '
                 f'fill="none" stroke="currentColor"/>\n')
    return (f'''    <g id="contSym" stroke-width="9" stroke-linecap="round">
{arcs}      <circle cx="{cx}" cy="{cy}" r="12" fill="currentColor" stroke="none"/>
    </g>
''')

def build_continuity(framed=False):
    defs = DEFS.replace("__EXTRA_DEFS__", continuity_def())
    frame = FRAME if framed else ""
    return HEAD + defs + BG + frame + symbol_uses("contSym") + TAIL

# ---------- Write files (plain + framed) ----------
builders = [
    ("Asterisk", build_text),
    ("4W", build_text),
    ("Diode", build_diode),
    ("Continuity", build_continuity),
]
def render(name, framed):
    if name == "4W":
        return build_text("4W", framed=framed)
    if name == "Asterisk":
        return build_text("*", framed=framed, fs=350, yoffset=75)
    if name == "Diode":
        return build_diode(framed=framed)
    return build_continuity(framed=framed)

for name, _ in builders:
    with open(f"{OUT}/symbol_glow_{name}.svg", "w") as f:
        f.write(render(name, False))
    with open(f"{OUT}/symbol_glow_{name}_framed.svg", "w") as f:
        f.write(render(name, True))

# ---------- Overviews (3 tiles in a row), plain and framed ----------
IDS = ["digit", "hugeGlow", "bigGlow", "midGlow", "diodeSym", "contSym"]
def inner_namespaced(svg, idx):
    inner = re.sub(r'^<svg[^>]*>', '', svg, flags=re.S).rsplit('</svg>', 1)[0]
    for nid in IDS:
        inner = inner.replace(f'id="{nid}"', f'id="{nid}_{idx}"')
        inner = inner.replace(f'url(#{nid})', f'url(#{nid}_{idx})')
        inner = inner.replace(f'href="#{nid}"', f'href="#{nid}_{idx}"')
    return inner

scale = 0.62
tileW, tileH = int(W*scale), int(H*scale)
gap = 14
mL = 18
cols = 4
sheetW = mL*2 + cols*tileW + (cols-1)*gap
sheetH = mL*2 + tileH

def build_sheet(path, framed):
    parts = [f'<svg viewBox="0 0 {sheetW} {sheetH}" xmlns="http://www.w3.org/2000/svg" '
             f'font-family="Arial, Helvetica, sans-serif">']
    parts.append(f'<rect x="0" y="0" width="{sheetW}" height="{sheetH}" fill="#000000"/>')
    for i, (name, _) in enumerate(builders):
        x = mL + i*(tileW+gap)
        y = mL
        parts.append(f'<g transform="translate({x},{y}) scale({scale})">')
        parts.append(inner_namespaced(render(name, framed), i))
        parts.append('</g>')
    parts.append('</svg>')
    with open(path, "w") as f:
        f.write("\n".join(parts))

build_sheet(f"{OUT}/symbol_glow_overview.svg", False)
build_sheet(f"{OUT}/symbol_glow_framed_overview.svg", True)

print("wrote 3 symbols x 2 versions + 2 overviews; canvas", W, "x", H, "sheet", sheetW, "x", sheetH)
