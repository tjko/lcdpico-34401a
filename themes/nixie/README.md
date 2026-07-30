# LCDpico "Nixie" theme for HP34401A


These are the scripts used to generate graphics for this theme (nixie1.png and nixie2.png).



## SVG Graphics

These SVG generator scripts were initially created by a LLM and then were modified as needed:

script|description
------|----------
gen_words.py|generate indicator words
gen_symbols.py|generat indicator symbols
get_nixie_alnum.py|generate character (main display numbers/characters)


When run these scripts save output SVG files under directory ```./outputs```


## PNG Conversion

These scripts were created to convert SVG images into PNG images that
are suitable to load into GPU video memory to be used as graphics tiles/textures.


script|description
------|-----------
gen_nixie_png.py|generate nixie1.png
gen_indicator_png.py|generate nixie2.png

NOTE! currently these scripts rely on (MacOS) command ```qalmanage``` to convert SVG into PNG image.
Thus script need to be modified to use some other conversion tool if not run on MacOS.







