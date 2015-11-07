## huh?

Tiny command-line tool to pack sprites and fonts into a spritesheet. Output is a PNG image (the spritesheet) and a XML description.

The font packer can also add simple effects like outlines, gradients and drop shadows.

## sample

![sample](/sample.png?raw=true "sample")

    ./packfont -w 600 -h 256 -s 36 -g 3 -d 2 -e 2 -B 2 -S .4 -i ffffff00-ffffffff-ffff0000 -o ff8f0000-ff3f3f00 DejaVuSans.ttf sample x61-x7a x41-x5a x30-x39

## usage

### packsprites

    usage: packsprites [options] sheetname spritepath

    options:
    -b		size of border around the packed sprites, in pixels (default: 2)
    -w		spritesheet width (default: 256)
    -h		spritesheet height (default: 256)


`sheetname` is the basename of the generated XML/PNG files, and `spritepath` is the path of a directory with the sprites to be packed.

### packfont

    usage: packfont [options] font sheetname range...

    options:
    -b		size in pixels of border around the packed sprites (default: 2)
    -w		spritesheet width (default: 256)
    -h		spritesheet height (default: 256)
    -s		font size (default: 16)
    -g		outline radius, in pixels (default: 2)
    -i		font color
    -o		outline color
    -S		drop shadow opacity, between 0 and 1 (default: .2)
    -B		drop shadow gaussian blur radius, in pixels (default: 0)
    -d		drop shadow x offset, in pixels (default: 0)
    -e		drop shadow y offset, in pixels (default: 0)

`font` is a path to a TrueType font, `sheetname` is the basename of the generated XML/PNG files, and `range` is a character range (e.g. `x30-x39`). Multiple character ranges are accepted.

## output format

TODO
