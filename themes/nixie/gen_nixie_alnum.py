#!/usr/bin/python3
#

import sys
import re
from pathlib import Path


# Uniform character sizing (same cap height for digits, letters, punctuation)
CH_FS = 250
CH_TF = "matrix(1,0,0,1.4,0,-15)"   # text y=200 -> visual center y=265
CH_TF_NUM = "matrix(1.4,0,0,1.7,-80,-100)"   # text y=200 -> visual center y=265
CH_TF_HYP = "matrix(1.4,0,0,1.7,-80,-120)"
CH_TF_U = "matrix(1.4,0,0,1.7,-80,-170)"

TEMPLATE = r'''<svg viewBox="0 0 400 640" xmlns="http://www.w3.org/2000/svg" font-family="Arial, Helvetica, sans-serif">
  <defs>
    <radialGradient id="bg" cx="50%" cy="42%" r="75%">
      <stop offset="0%" stop-color="#181821"/>
      <stop offset="60%" stop-color="#0e0e15"/>
      <stop offset="100%" stop-color="#070709"/>
    </radialGradient>
    <radialGradient id="halo" cx="50%" cy="50%" r="50%">
      <stop offset="0%" stop-color="#ffa540" stop-opacity="0.72"/>
      <stop offset="40%" stop-color="#ff6e12" stop-opacity="0.40"/>
      <stop offset="80%" stop-color="#ff5e00" stop-opacity="0.11"/>
      <stop offset="100%" stop-color="#ff5e00" stop-opacity="0"/>
    </radialGradient>
    <linearGradient id="glass" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#bcd2e0" stop-opacity="0.22"/>
      <stop offset="20%" stop-color="#dff0fb" stop-opacity="0.10"/>
      <stop offset="50%" stop-color="#9fb6c6" stop-opacity="0.04"/>
      <stop offset="80%" stop-color="#7c93a4" stop-opacity="0.08"/>
      <stop offset="100%" stop-color="#5d7280" stop-opacity="0.18"/>
    </linearGradient>
    <linearGradient id="glassEdge" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#d8ecf7" stop-opacity="0.7"/>
      <stop offset="50%" stop-color="#90a8b8" stop-opacity="0.25"/>
      <stop offset="100%" stop-color="#4f636f" stop-opacity="0.6"/>
    </linearGradient>
    <linearGradient id="base" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#3a3a40"/>
      <stop offset="35%" stop-color="#23232a"/>
      <stop offset="100%" stop-color="#0e0e12"/>
    </linearGradient>
    <linearGradient id="collar" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#5a5a62"/>
      <stop offset="50%" stop-color="#2e2e35"/>
      <stop offset="100%" stop-color="#15151a"/>
    </linearGradient>
    <linearGradient id="pin" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#6b5a2c"/>
      <stop offset="45%" stop-color="#d9bd6e"/>
      <stop offset="55%" stop-color="#f3e3a8"/>
      <stop offset="100%" stop-color="#7a6630"/>
    </linearGradient>
    <linearGradient id="digit" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#ffeec2"/>
      <stop offset="45%" stop-color="#ffb449"/>
      <stop offset="100%" stop-color="#ff860f"/>
    </linearGradient>
    <pattern id="mesh" width="13" height="13" patternUnits="userSpaceOnUse">
      <path d="M13 0 V13 M0 13 H13" stroke="#d4e6f2" stroke-width="0.9" fill="none"/>
    </pattern>
    <pattern id="meshDark" width="13" height="13" patternUnits="userSpaceOnUse">
      <path d="M12.2 0 V13 M0 12.2 H13" stroke="#1c232a" stroke-width="1.1" fill="none"/>
    </pattern>
    <filter id="hugeGlow" x="-120%" y="-120%" width="340%" height="340%">
      <feGaussianBlur stdDeviation="16"/>
    </filter>
    <filter id="bigGlow" x="-90%" y="-90%" width="280%" height="280%">
      <feGaussianBlur stdDeviation="9"/>
    </filter>
    <filter id="midGlow" x="-50%" y="-50%" width="200%" height="200%">
      <feGaussianBlur stdDeviation="3"/>
    </filter>
    <clipPath id="tubeClip">
      <path d="M72 442 L72 135 C72 70 128 40 200 40 C272 40 328 70 328 135 L328 442 Z"/>
    </clipPath>
  </defs>

  <rect x="0" y="0" width="400" height="640" fill="url(#bg)"/>
  <ellipse cx="200" cy="255" rx="128" ry="160" fill="url(#halo)" opacity="__HALO__"/>

  <g clip-path="url(#tubeClip)">
    <g fill="#52473b" opacity="0.26" text-anchor="middle" font-size="__FS__">
      <text x="186" y="205" dominant-baseline="central" transform="__TF__">__A__</text>
    </g>
    <g fill="#473d33" opacity="0.18" text-anchor="middle" font-size="__FS__">
      <text x="214" y="195" dominant-baseline="central" transform="__TF__">__B__</text>
    </g>
    <g fill="#3f362d" opacity="0.15" text-anchor="middle" font-size="__FS__">
      <text x="200" y="210" dominant-baseline="central" transform="__TF__">__C__</text>
    </g>

    <text x="200" y="200" dominant-baseline="central" text-anchor="middle" transform="__TF__"
          font-size="__FS__" fill="#ff7414" opacity="0.62" filter="url(#hugeGlow)">__D__</text>
    <text x="200" y="200" dominant-baseline="central" text-anchor="middle" transform="__TF__"
          font-size="__FS__" fill="#ff7e1e" opacity="0.88" filter="url(#bigGlow)">__D__</text>
    <text x="200" y="200" dominant-baseline="central" text-anchor="middle" transform="__TF__"
          font-size="__FS__" fill="#ffa838" opacity="0.95" filter="url(#midGlow)">__D__</text>
    <text x="200" y="200" dominant-baseline="central" text-anchor="middle" transform="__TF__"
          font-size="__FS__" fill="url(#digit)" stroke="#fff1cf" stroke-width="1.4">__D__</text>

    __DOT__

    <rect x="60" y="40" width="280" height="410" fill="url(#meshDark)" opacity="0.30"/>
    <rect x="60" y="40" width="280" height="410" fill="url(#mesh)" opacity="0.22"/>
  </g>

  <path d="M72 442 L72 135 C72 70 128 40 200 40 C272 40 328 70 328 135 L328 442 Z"
        fill="url(#glass)" stroke="url(#glassEdge)" stroke-width="2.2"/>
  <path d="M95 140 C92 95 118 62 150 56 C132 80 122 110 124 160 C126 250 124 350 130 430 L108 430 C100 340 98 230 95 140 Z"
        fill="#ffffff" opacity="0.10"/>
  <path d="M300 150 C306 250 304 350 300 432 L312 432 C316 340 318 240 312 150 Z"
        fill="#ffffff" opacity="0.05"/>

  <ellipse cx="200" cy="36" rx="11" ry="15" fill="url(#glass)" stroke="url(#glassEdge)" stroke-width="1.5"/>
  <ellipse cx="200" cy="24" rx="4" ry="7" fill="#9fb6c6" opacity="0.5"/>

  <g stroke="url(#pin)" stroke-width="5" stroke-linecap="round">
    <line x1="138" y1="524" x2="132" y2="586"/>
    <line x1="156" y1="528" x2="152" y2="595"/>
    <line x1="175" y1="530" x2="173" y2="600"/>
    <line x1="194" y1="531" x2="193" y2="602"/>
    <line x1="213" y1="531" x2="215" y2="602"/>
    <line x1="232" y1="530" x2="236" y2="600"/>
    <line x1="251" y1="528" x2="257" y2="595"/>
    <line x1="269" y1="524" x2="277" y2="586"/>
  </g>

  <rect x="74" y="430" width="252" height="26" rx="7" fill="url(#collar)"/>
  <ellipse cx="200" cy="431" rx="126" ry="13" fill="#454550"/>
  <ellipse cx="200" cy="431" rx="126" ry="13" fill="#ff7a18" opacity="0.08"/>

  <path d="M86 452 L314 452 L304 524 Q200 540 96 524 Z" fill="url(#base)"/>
  <ellipse cx="200" cy="452" rx="114" ry="16" fill="#2b2b32"/>
  <ellipse cx="200" cy="452" rx="114" ry="16" fill="none" stroke="#46464f" stroke-width="1.2"/>
  <ellipse cx="200" cy="524" rx="104" ry="13" fill="#0a0a0d"/>
  <path d="M96 460 Q130 470 130 510 L120 512 Q110 478 96 466 Z" fill="#5a5a62" opacity="0.25"/>
</svg>
'''

OUT = "./outputs"
IDS = ["bg","halo","glass","glassEdge","base","collar","pin","digit",
       "mesh","meshDark","hugeGlow","bigGlow","midGlow","tubeClip"]

# Ordered character set: digits, A-Z, common punctuation, space
DIGITS = list("0123456789")
LETTERS = list("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
PUNCT = [
    (".", "period"), (",", "comma"), (":", "colon"), (";", "semicolon"),
    ("!", "exclam"), ("?", "question"), ("'", "apostrophe"), ('"', "dquote"),
    ("-", "hyphen"), ("^", "tilde"), ("(", "lparen"), (")", "rparen"),
    ("[", "lbracket"), ("]", "rbracket"), ("/", "slash"), ("\\", "backslash"),
    ("@", "at"), ("#", "hash"), ("$", "dollar"), ("%", "percent"),
    ("&", "ampersand"), ("*", "asterisk"), ("+", "plus"), ("=", "equals"),
    ("<", "less"), (">", "greater"), ("_", "underscore"),
]
SPACE = [(" ", "space")]

# Build ordered (char, name) list
ORDER = [(d, d) for d in DIGITS] + [(l, l) for l in LETTERS] + [(l.lower(),l.lower() + '_lc') for l in LETTERS] + PUNCT + SPACE
N = len(ORDER)

# Faint background pool (clean glyphs only)
FAINT_POOL = list("0123456789")
#FAINT_POOL = list("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ")
FP = len(FAINT_POOL)

out_path = Path(OUT)
if not out_path.is_dir():
    sys.exit(f"Cannot find directory: {OUT}")


def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

LIT_DOT = '''<!-- Lit decimal dot -->
    <circle cx="311" cy="385" r="15" fill="#ff7414" opacity="0.62" filter="url(#hugeGlow)"/>
    <circle cx="311" cy="385" r="12.5" fill="#ff7e1e" opacity="0.88" filter="url(#bigGlow)"/>
    <circle cx="311" cy="385" r="10.5" fill="#ffa838" opacity="0.95" filter="url(#midGlow)"/>
    <circle cx="311" cy="385" r="9.5" fill="url(#digit)" stroke="#fff1cf" stroke-width="1.2"/>'''

DARK_DOT = '''<!-- Unlit decimal dot (dark) -->
    <circle cx="311" cy="385" r="9.5" fill="#2b2620" opacity="0.9"/>
    <circle cx="311" cy="385" r="9.5" fill="none" stroke="#4a4034" stroke-width="1.1" opacity="0.6"/>'''


LIT_DOT2 = '''<!-- Lit decimal dot -->
    <circle cx="311" cy="185" r="15" fill="#ff7414" opacity="0.62" filter="url(#hugeGlow)"/>
    <circle cx="311" cy="185" r="12.5" fill="#ff7e1e" opacity="0.88" filter="url(#bigGlow)"/>
    <circle cx="311" cy="185" r="10.5" fill="#ffa838" opacity="0.95" filter="url(#midGlow)"/>
    <circle cx="311" cy="185" r="9.5" fill="url(#digit)" stroke="#fff1cf" stroke-width="1.2"/>'''


LIT_DOT3 = '''<!-- Lit "comma" dot -->
    <circle cx="309" cy="399" r="12" fill="#ff7414" opacity="0.62" filter="url(#hugeGlow)"/>
    <circle cx="309" cy="399" r="9.5" fill="#ff7e1e" opacity="0.88" filter="url(#bigGlow)"/>
    <circle cx="305" cy="399" r="8.5" fill="#ffa838" opacity="0.95" filter="url(#midGlow)"/>
    <circle cx="305" cy="399" r="7.5" fill="url(#digit)" stroke="#fff1cf" stroke-width="1.2"/>'''



def make_tube(i, dot="dark", dot2="dark"):
    ch = ORDER[i][0]
    is_space = (ch == " ")
    lit_dot = LIT_DOT
    if dot2 != "dark":
        lit_dot = LIT_DOT2 + LIT_DOT3 + LIT_DOT
    a = FAINT_POOL[(i*3 + 5) % FP]
    b = FAINT_POOL[(i*3 + 13) % FP]
    c = FAINT_POOL[(i*3 + 23) % FP]
    main = "" if is_space else esc(ch)
    halo = "0" if is_space else "1"   # no central glow for a blank (space) tube
    return (TEMPLATE
            .replace("__FS__", str(CH_FS))
            .replace("__TF__", CH_TF_NUM if (ch.isdigit()) else CH_TF_U if (ch == '_') else CH_TF_HYP if (ch == '-') else  CH_TF)
            .replace("__HALO__", halo)
            .replace("__DOT__", lit_dot if dot == "lit" else DARK_DOT)
            .replace("__A__", esc(a))
            .replace("__B__", esc(b))
            .replace("__C__", esc(c))
            .replace("__D__", main))

def safe_name(i):
    name = ORDER[i][1]
    # digits/letters keep their character; punctuation uses words
    return name

# Write individual files (dark-dot keeps original name; lit-dot gets _dot_on)
paths_dark, paths_lit = [], []
for i in range(N):
    base = safe_name(i)
    fd = f"{OUT}/nixie_alnum_{base}.svg"
    fl = f"{OUT}/nixie_alnum_{base}_dot_on.svg"
    fl2 = f"{OUT}/nixie_alnum_{base}_2dot_on.svg"
    with open(fd, "w") as f:
        f.write(make_tube(i, dot="dark"))
    with open(fl, "w") as f:
        f.write(make_tube(i, dot="lit"))
    if base == "space":
        with open(fl2, "w") as f:
            f.write(make_tube(i, dot="lit", dot2="lit"))
        paths_lit.append(fl2)
    paths_dark.append(fd)
    paths_lit.append(fl)

# Overview sheets: 9 cols x 7 rows
def inner_namespaced(i, idx, dot="dark"):
    svg = make_tube(i, dot=dot)
    inner = re.sub(r'^<svg[^>]*>', '', svg).rsplit('</svg>', 1)[0]
    for nid in IDS:
        inner = inner.replace(f'id="{nid}"', f'id="{nid}_{idx}"')
        inner = inner.replace(f'url(#{nid})', f'url(#{nid}_{idx})')
    return inner

scale = 0.34
tileW, tileH = int(400*scale), int(640*scale)
gap = 10
mL = 18
cols = 9
rows = (N + cols - 1) // cols
sheetW = mL*2 + cols*tileW + (cols-1)*gap
sheetH = mL*2 + rows*tileH + (rows-1)*gap

def build_sheet(path, dot="dark"):
    parts = [f'<svg viewBox="0 0 {sheetW} {sheetH}" xmlns="http://www.w3.org/2000/svg" '
             f'font-family="Arial, Helvetica, sans-serif">']
    parts.append(f'<rect x="0" y="0" width="{sheetW}" height="{sheetH}" fill="#070709"/>')
    for i in range(N):
        col, row = i % cols, i // cols
        x = mL + col*(tileW+gap)
        y = mL + row*(tileH+gap)
        parts.append(f'<g transform="translate({x},{y}) scale({scale})">')
        parts.append(inner_namespaced(i, i, dot=dot))
        parts.append('</g>')
    parts.append('</svg>')
    with open(path, "w") as f:
        f.write("\n".join(parts))

ov_dark = f"{OUT}/nixie_alnum_overview.svg"
ov_lit = f"{OUT}/nixie_alnum_dot_on_overview.svg"
build_sheet(ov_dark, dot="dark")
build_sheet(ov_lit, dot="lit")

print(f"characters: {N}")
print(f"sheet: {sheetW} x {sheetH}, grid {cols}x{rows}")
print("OVERVIEW_LIT:" + ov_lit)
for p in paths_lit:
    print("FILE:" + p)
