# Cloudy Climb
Cloudy Climb is a short open-source platformer made from scratch in in 2 months. It's a 're-imagining' of sorts, as I made a much worse game with the same name and premise a few years ago. I was in the mood to make a little game recently, so I decided to do the concept justice. The result is unlikely to blow your mind, but I think it can provide a few minutes of entertainment, at least.

There's also a standalone level editor, which is pretty cool I guess.

If you're curious about the development, you can read the [postmortem](http://www.huguesross.net/2018/03/cloudy-climb-announcement-and.html).

## Controls
### Menu
- Left/Right: Change selection/Adjust volume
- Z/Space:    Select/Confirm
- Escape:     Back/Quit

### Ingame
- Left/Right: Move
- Z/Space:    Bounce
- Escape:     Pause

## How to Play
At the start of every level, press the *Bounce* key to begin. Your character
will bounce off of clouds automatically. To bounce higher, you can press the
*Bounce* key just as they land on a cloud.

Every level contains a star. Stars do nothing, but can provide an extra
challenge in some levels. The game will track how many you've collected, and
you can see which ones you're missing in the levels menu.

The rest of the game is fairly self-explanatory. Good Luck, and have fun!

## Building
To build Cloudy Climb, you'll need to build and install a copy of [DFGame](https://github.com/Df458/dfgame) built with libPNG support.
```bash
# Create build directory and generate project files
mkdir build && cd build
meson .. --buildtype=release

# Compile and install
ninja
ninja install
```
