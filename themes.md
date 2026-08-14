# LCDpico-34401A Themes

Currently included themes are following:


Theme|Sample Image|Notes
-----|------------|-----
nixie|<img src="images/fp-nixie.png" width="300">|Initial them with "bold" font.
nixiethin|<img src="images/fp-nixiethin.png" width="300">|More realistic Nixie tube renderings.


## Theme Graphics

### nixie

![alnum](src/img/240x960/nixie1.png)
![symbols](src/img/240x960/nixie2.png)

### nixiethin

![alnum](src/img/240x960/nixiethin1.png)
![symbols](src/img/240x960/nixie2.png)


## Selecting Theme

Theme can be select at compile time by setting ```LCDPICO_THEME``` setting.


For example:

```
$ cd build
$ cmake -DLCDPIO_THEME=nixiethin ..
$ make -j $(nproc)
```

