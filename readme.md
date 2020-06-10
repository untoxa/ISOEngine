# Simple "2.5D" engine for game boy

This is a simple engine that allows to make "pseudo-3D" games for Nintendo Game Boy using
GBDK-2020.

![screenshot](/screenshot.gif)

You need the latest version of GBDK-2020 (3.2 for now) to compile this example. I'm also
using the tool from ZGB engine to convert the "Gameboy Tile Designer" tiles into the source 
file and a tool to convert the description of the scene into the source file (tool source 
is included).

## Maps
The global world map consists of "rooms". Each room has a 9x9x4 cell 3d-map: four 9x9 2d 
layers. The rooms and the "world map" are defined in the rooms.3dmap file, which is a text
file with "ini-file" format. Comments in that file describe it's internal structure. This
file is converted into the source file by the "mapcvt" utility at compile-time. Rooms and 
global world are translated into the C linked-list structures. That allows to save a lot 
of space and accelerate the drawing functions. Collision map for the current room is 
restored into ram from that lists when the room is changed. Maximum number of items in the 
room is 254.

## Room items
Each room item consist of two 16x16 pixel "sprites", the first one is the item itself, and
the second one is a bit mask, which defines what pixels are transparent. When the item is 
drawn, the part of the background is AND'ed with the mask, and then OR'ed with the sprite.
The graphics is in GBTD format (file: scene_resources.b1.gbr), and is converted into the 
source file at compile time by the "gbr2c" utility.


Tony.