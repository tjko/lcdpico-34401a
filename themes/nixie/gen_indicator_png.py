#!/usr/bin/python3
#

import sys
import re
import subprocess
from pathlib import Path


IN = "./outputs"
OUT = "./png"



WORDS = ["Adrs", "Rmt", "Man", "Trig", "Hold", "Mem", "Ratio", "Math", "ERROR", "Rear", "Shift"]
SYMBOLS = [ "4W", "Diode", "Continuity", "Asterisk" ]



LIST = [ "word_glow_" + item for item in WORDS ] + [ "symbol_glow_" + item for item in SYMBOLS ]


in_path = Path(IN)
out_path = Path(OUT)

if not in_path.is_dir():
    sys.exit(f"Cannot find directory: {IN}")

if not out_path.is_dir():
    sys.exit(f"Cannot find directory: {OUT}")




# convert SVGs to PNGs

infiles = []

for i in LIST:
    if "Asterisk" in i:
        fname = f"{IN}/{i}.svg"
        oname = f"{OUT}/{i}.svg.png"
    else:
        fname = f"{IN}/{i}_framed.svg"
        oname = f"{OUT}/{i}_framed.svg.png"
    print(fname)
    res = subprocess.run(["qlmanage", "-t", "-s", "1024", "-o", OUT, fname])
    if not Path(oname).is_file():
        sys.exit(f"cannot find: {oname}")
    nname = f"{OUT}/{i}_scaled.png"

    print(f"Resize: {oname}")
#    res = subprocess.run(["magick",oname,"-crop","432x820+296+24","-rotate","-90","-resize","200x76!","+repage",nname])
    if "word" in oname:
        res = subprocess.run(["magick",oname,"-crop","636x264+64+380","+repage","-rotate","-90","-resize","38x80!","+repage",nname])
    else:
        res = subprocess.run(["magick",oname,"-crop","894x516+64+254","+repage","-rotate","-90","-resize","38x64!","+repage","-background","black","-gravity","north","-extent","38x80",nname])

    if not Path(nname).is_file():
        sys.exit(f"cannot find: {nname}")

    infiles.append(nname)


# create combined PNG image



res = subprocess.run(["magick","montage"] +  infiles + ["-tile", "8x", "-geometry", "+0+0", "-background", "black", "nixie2.png"])
