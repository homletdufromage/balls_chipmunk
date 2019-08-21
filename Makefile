# ----------
# commands
# ----------
CC=g++
LD=g++

# ----------
# paths
# ----------
SOURCES=sources

# ----------
# flags
# ----------
CPPFLAGS=--pedantic -Wall -W -Wno-unused-parameter
LDFLAGS=-lSDL2 -lSDL2_image -lSDL2_ttf -lchipmunk  -lSDL2_gfx -L$(SOURCES)
#-lSDL2_gfx

# ----------
# objects
# ----------
EXEC=game
OBJECTS=main.o Texture.o Ball.o

# ----------
# Game
# ----------
all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(LD) -o $(EXEC) $(OBJECTS) $(LDFLAGS)

main.o: $(SOURCES)/main.cpp
	$(CC) -c $(SOURCES)/main.cpp -o main.o $(CPPFLAGS)

Texture.o: $(SOURCES)/Texture.cpp $(SOURCES)/Texture.h
	$(CC) -c $(SOURCES)/Texture.cpp -o Texture.o $(CPPFLAGS)

Ball.o: $(SOURCES)/Ball.cpp $(SOURCES)/Ball.h
	$(CC) -c $(SOURCES)/Ball.cpp -o Ball.o $(CPPFLAGS)

clean:
	rm -f *.o
	rm $(EXEC)
	clear
