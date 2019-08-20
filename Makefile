# ----------
# commands
# ----------
CC=g++
LD=g++

# ----------
# flags
# ----------
CPPFLAGS=--pedantic -Wall -W -Wno-unused-parameter
LDFLAGS=-lSDL2 -lSDL2_image -lchipmunk  -lSDL2_gfx -L.
#-lSDL2_gfx

# ----------
# objects
# ----------
EXEC=game
OBJECTS=main.o

# ----------
# paths
# ----------
GAME_PATH=.
INCLUDES=$(GAME_PATH)

# ----------
# Game
# ----------
all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(LD) -o $(EXEC) $(OBJECTS) $(LDFLAGS)

main.o: main.cpp
	$(CC) -c main.cpp -o main.o $(CPPFLAGS) $(LDFLAGS)

clean:
	rm -f *.o
	rm $(EXEC)
	clear
