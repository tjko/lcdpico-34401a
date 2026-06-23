#!/usr/bin/python3
#

import sys
import re
import subprocess
from pathlib import Path


IN = "./outputs"
OUT = "./png"

PREFIX = "nixie_alnum_"


DIGITS = list("0123456789")
LETTERS = list("ABCDEFGHIJKLMNOPQRSTUVWXYZ")


LIST = DIGITS + LETTERS + [ "period", "comma", "colon", "semicolon", "exclam", "question", "apostrophe", "dquote", "hyphen", "tilde", "hash", "asterisk", "plus", "equals", "less", "greater", "space", "space_dot_on", "space_2dot_on"  ]



in_path = Path(IN)
out_path = Path(OUT)

if not in_path.is_dir():
    sys.exit(f"Cannot find directory: {IN}")

if not out_path.is_dir():
    sys.exit(f"Cannot find directory: {OUT}")


# convert SVGs to PNGs

infiles = []

for i in LIST:
    fname = f"{IN}/{PREFIX}{i}.svg"
    print(fname)
    res = subprocess.run(["qlmanage", "-t", "-s", "1024", "-o", OUT, fname])
    oname = f"{OUT}/{PREFIX}{i}.svg.png"
    nname = f"{OUT}/{PREFIX}{i}.png"
    if not Path(oname).is_file():
        sys.exit(f"cannot find: {oname}")

    print(f"Resize: {oname}")
#    res = subprocess.run(["magick",oname,"-crop","432x820+296+24","-rotate","-90","-resize","200x76!","+repage",nname])
    res = subprocess.run(["magick",oname,"-crop","432x760+296+24","-rotate","-90","-resize","190x80!","+repage",nname])
    if not Path(nname).is_file():
        sys.exit(f"cannot find: {nname}")

    infiles.append(nname)



# create combined PNG image



res = subprocess.run(["magick","montage"] +  infiles + ["-tile", "4x", "-geometry", "+0+0", "nixie1.png"])
