#!/usr/bin/python3
#

import sys
import re
from pathlib import Path



OUT = "./outputs"

out_path = Path(OUT)
if not out_path.is_dir():
    sys.exit(f"Cannot find directory: {OUT}")


WORDS = ["Adrs", "Rmt", "Man", "Trig", "Hold", "Mem", "Ratio", "Math", "ERROR", "Rear", "Shift"]
FS = 170  # uniform font size; all listed words fit the 820-wide canvas

TEMPLATE = r'''<svg viewBox="0 0 830 300" xmlns="http://www.w3.org/2000/svg" font-family="Arial, Helvetica, sans-serif">
  <defs>
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
  </defs>

  <rect x="0" y="0" width="820" height="300" fill="#000000"/>
__FRAME__

  <text x="310" y="152" dominant-baseline="central" text-anchor="middle"
        font-size="__FS__" fill="#ff7414" opacity="0.6" filter="url(#hugeGlow)">__W__</text>
  <text x="310" y="152" dominant-baseline="central" text-anchor="middle"
        font-size="__FS__" fill="#ff7e1e" opacity="0.88" filter="url(#bigGlow)">__W__</text>
  <text x="310" y="152" dominant-baseline="central" text-anchor="middle"
        font-size="__FS__" fill="#ffa838" opacity="0.95" filter="url(#midGlow)">__W__</text>
  <text x="310" y="152" dominant-baseline="central" text-anchor="middle"
        font-size="__FS__" fill="url(#digit)" stroke="#fff1cf" stroke-width="1.4">__W__</text>
</svg>
'''

IDS = ["digit", "hugeGlow", "bigGlow", "midGlow"]

FRAME = '''
  <!-- Frame: fixed size in every image (centered 700x200, r=32) -->
  <rect x="60" y="50" width="500" height="200" rx="32" ry="32" fill="none"
        stroke="#ff7e1e" stroke-width="7" opacity="0.6" filter="url(#bigGlow)"/>
  <rect x="60" y="50" width="500" height="200" rx="32" ry="32" fill="none"
        stroke="#ffb84e" stroke-width="3"/>
  <rect x="60" y="50" width="500" height="200" rx="32" ry="32" fill="none"
        stroke="#fff1cf" stroke-width="1" opacity="0.65"/>'''

def make_word(w, framed=False):
    return (TEMPLATE
            .replace("__FS__", str(FS-40) if w == "ERROR" else str(FS))
            .replace("__FRAME__", FRAME if framed else "")
            .replace("__W__", w))

# Individual files: plain (no frame) and framed
for w in WORDS:
    with open(f"{OUT}/word_glow_{w}.svg", "w") as f:
        f.write(make_word(w, framed=False))
    with open(f"{OUT}/word_glow_{w}_framed.svg", "w") as f:
        f.write(make_word(w, framed=True))

# Overview sheet: 2 columns x 6 rows, ids namespaced per tile
def inner_namespaced(w, idx, framed=False):
    svg = make_word(w, framed=framed)
    inner = re.sub(r'^<svg[^>]*>', '', svg).rsplit('</svg>', 1)[0]
    for nid in IDS:
        inner = inner.replace(f'id="{nid}"', f'id="{nid}_{idx}"')
        inner = inner.replace(f'url(#{nid})', f'url(#{nid}_{idx})')
    return inner

scale = 0.5
tileW, tileH = int(820*scale), int(300*scale)  # 410 x 150
gap = 12
mL = 18
cols = 2
rows = (len(WORDS) + cols - 1) // cols
sheetW = mL*2 + cols*tileW + (cols-1)*gap
sheetH = mL*2 + rows*tileH + (rows-1)*gap

def build_sheet(path, framed=False):
    parts = [f'<svg viewBox="0 0 {sheetW} {sheetH}" xmlns="http://www.w3.org/2000/svg" '
             f'font-family="Arial, Helvetica, sans-serif">']
    parts.append(f'<rect x="0" y="0" width="{sheetW}" height="{sheetH}" fill="#000000"/>')
    for i, w in enumerate(WORDS):
        col, row = i % cols, i // cols
        x = mL + col*(tileW+gap)
        y = mL + row*(tileH+gap)
        parts.append(f'<g transform="translate({x},{y}) scale({scale})">')
        parts.append(inner_namespaced(w, i, framed=framed))
        parts.append('</g>')
    parts.append('</svg>')
    with open(path, "w") as f:
        f.write("\n".join(parts))

build_sheet(f"{OUT}/word_glow_overview.svg", framed=False)
build_sheet(f"{OUT}/word_glow_framed_overview.svg", framed=True)

print("words:", len(WORDS), "-> 2 versions each; sheet:", sheetW, "x", sheetH)
