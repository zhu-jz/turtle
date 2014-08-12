
# Turtle Makefile

all: turtle

clean:
	$(RM) *.o *~ core

# General

CC      = gcc
CFLAGS  = -pipe
LDFLAGS = -lm

# X11

LDFLAGS += -L/usr/X11R6/lib -lX11

# Solaris

# LDFLAGS += -lnsl -lsocket

# Warnings

CFLAGS  += -ansi # -Wall -pedantic -Winline

# Optimize

CFLAGS  += -O2 -fforce-mem -fomit-frame-pointer -funroll-loops
LDFLAGS += -s

# Debug

# CFLAGS  += -g

# Profile

# CFLAGS  += -pg
# LDFLAGS += -pg

# Dependencies

OBJS = board.o book.o clock.o database.o eboard.o endgame.o eval.o filesys.o \
       game.o hash.o ios.o loop.o master.o mboard.o midgame.o output.o \
       probcut.o sort.o symmetry.o turtle.o variable.o xboard.o

turtle: $(OBJS)
	$(CC) $(LDFLAGS) -o turtle $(OBJS)

board.o: board.c board.h types.h output.h

book.o: book.c book.h types.h board.h clock.h eval.h game.h loop.h master.h \
        mboard.h output.h symmetry.h

clock.o: clock.c clock.h

database.o: database.h types.h board.h output.h

eboard.o: eboard.c eboard.h types.h board.h hash.h output.h

endgame.o: endgame.c endgame.h types.h board.h clock.h eboard.h eval.h hash.h \
           hash.i loop.h master.h mboard.h output.h sort.h

eval.o: eval.c eval.h types.h board.h mboard.h output.h

filesys.o: filesys.c filesys.h types.h

game.o: game.c game.h types.h board.h clock.h output.h

hash.o: hash.c hash.h types.h board.h eval.h output.h

ios.o: ios.c ios.h types.h

loop.o: loop.c loop.h types.h board.h book.h filesys.h game.h hash.h ios.h \
        master.h mboard.h output.h turtle.h variable.h xboard.h

master.o: master.c master.h types.h board.h book.h clock.h database.h \
          endgame.h eval.h game.h hash.h loop.h mboard.h midgame.h output.h \
          sort.h

mboard.o: mboard.c mboard.h types.h board.h eval.h hash.h

midgame.o: midgame.c midgame.h types.h board.h clock.h eval.h hash.h hash.i \
           loop.h master.h mboard.h mboard.i probcut.h sort.h

output.o: output.c output.h types.h board.h clock.h game.h xboard.h

probcut.o: probcut.c probcut.h types.h eval.h output.h

sort.o: sort.c sort.h board.h eval.h mboard.h

symmetry.o: symmetry.c symmetry.h board.h

turtle.o: turtle.c turtle.h types.h board.h book.h clock.h database.h \
          eboard.h endgame.h eval.h game.h hash.h loop.h master.h mboard.h \
          output.h probcut.h variable.h xboard.h

variable.o: variable.c variable.h types.h board.h book.h clock.h endgame.h \
            filesys.h game.h hash.h ios.h loop.h master.h mboard.h midgame.h \
            output.h probcut.h turtle.h

xboard.o: xboard.c xboard.h types.h board.h output.h turtle.h

