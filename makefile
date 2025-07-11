# Nom du compilateur
CC = gcc

# Options d'inclusion (drapeaux -I) et de liaison (drapeaux -l et -L)
INCLUDE = `sdl2-config --cflags` -I/usr/local/include
LIBS = `sdl2-config --libs` -lSDL2_image -ltmx -lz `xml2-config --libs` -lm


# Fichiers sources
SRC = main.c \
      framework/map.c framework/sprite.c game/entity.c game/player.c systems/utils.c systems/inputs.c game/pnj.c systems/camera.c  game/game.c

# Objets correspondants
OBJ = $(SRC:.c=.o)

# Nom de l'exécutable
EXEC = sdlapp

# Règle par défaut
all: $(EXEC)

# Édition des liens
$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

# Compilation des fichiers .c en .o
%.o: %.c
	$(CC) -c $< -o $@ $(INCLUDE)

# Nettoyage
clean:
	rm -f $(OBJ) $(EXEC)

# Exécution
run: $(EXEC)
	./$(EXEC)
