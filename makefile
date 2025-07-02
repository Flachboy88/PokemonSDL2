# Nom du compilateur
CC = gcc

# Options d'inclusion (drapeaux -I) et de liaison (drapeaux -l et -L)
INCLUDE = `sdl2-config --cflags` -I/usr/include/libxml2
LIBS = `sdl2-config --libs` -lSDL2_image -lxml2

# Fichiers sources
SRC = main.c framework/map.c framework/sprite.c resources/lib/cJSON.c
OBJ = $(SRC:.c=.o)

# Nom de l'exécutable
EXEC = sdlapp

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(SRC) -o $(EXEC) $(INCLUDE) $(LIBS)

clean:
	rm -f *.o $(EXEC)

run: $(EXEC)
	./$(EXEC)
