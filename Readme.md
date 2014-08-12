### Compiling

Go to the "src/" subdirectory, and just type "make". Note that GNU Make may
be needed because of "+=". If you want to compile Turtle without X11 (pure
ANSI C), overwrite the source file named "xboard.c" with "xboard_dummy.c"
before compiling. You also need to modify the Makefile so that the X11 library
is not linked with Turtle anymore (comment the line
"LDFLAGS += -L/usr/X11R6/lib -lX11").

### Command line

"turtle -s <file>" launches Turtle with <file> as settings file (located in
the "settings/" subdirectory. "turtle" alone will be interpreted as
"turtle -s default". Remember to always launch Turtle from its own directory
(not from the "bin/" subdirectory).

### Commands

Type "H" after the prompt to obtain command list. If you use the X11 board as
interface, only the one-letter commands are available ...

### Settings files

default: ASCII interface (with additionnal X11 board if compiled with Xlib)
gui:     the X11 board becomes the interface (advantage: non-blocking)
ldraw:   book self learning (you can choose a line by adding
         "Set LearnLine F5F6" for example, always start with F5 ...)
lnodraw: same as ldraw but Turtle will avoid the draw
nodraw:  same as default but Turtle will avoid the draw

### Variables

You can edit settings file or create your own one. Here is a brief list
of available variables:
--- | ---
**Mode**         | Play/BookLearn
**Default**      | Console/GUI/FileSystem/Turtle (interface type)
**Blocking**     | Boolean (is the interface blocking?)
**SigIntQuit**   | Boolean (should Turtle quit after ^C? If not, it only breaks
                           the search => kind of "force move" command)
**InPipe**       | String (input when using FileSystem)
**OutPipe**      | String (output when using FileSystem)
**UseBook**      | Boolean
**BookFileName** | String
**DrawValue**    | Integer (draw value for white)
**SwitchValue**  | Boolean (should DrawValue be reversed after each game? Useful
                           for self learning)
**LearnGame**    | Boolean (should the new games in book be learnt? different than 
                           StoreGame below)
**LearningTime** | Integer (seconds per move)
**TimeMode**     | Real/CPU
**GameTime**     | Integer (seconds for the whole game)
**Trust**        | Boolean (trust opponent time?)
**RestoreClocks** | Boolean (restore clocks when taking back moves?)
**ClockWise**    | Forward/Backward (display time from 0 or GameTime?)
**HashBits**     | Integer (number of bits in keys, needs 2^(HashBits+3) bytes)
**UseOppTime**   | Boolean (think on opponent time? Interface needs to be
                      non-blocking)
**UseProbCut**   | Boolean
**ProbCutLevel** | Float   (confidence interval)
**WldSolve**     | WLD/LD/DW (which window should be used first)
**FullSolve**    | Full/Window/NegaC*/Pessimism
**MgEventPeriod** | Integer (periodicity of event checks during midgame)
**EgEventPeriod** | Integer (periodicity of event checks during endgame)
**StoreGame**    | None/Loss/All (store game in book when finished?)
**AutoRestart**  | Boolean (start another game automatically?)
**AutoSwap**     | Boolean (switch colours after each game?)
**AutoSaveGame** | Boolean (auto save the game in ASCII in the log/ subdirectory?)

### Author

Xann (Fabien Letouzey)

email:     xann@melting-pot.org
home page: http://www.lifl.fr/~letouzey/

