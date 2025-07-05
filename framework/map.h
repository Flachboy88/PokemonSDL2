#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>
#include <tmx.h>
#include <stdbool.h>

// Structure pour stocker les informations de la carte
typedef struct
{
    tmx_map *tmx_map;
    float default_x_spawn;
    float default_y_spawn;
    // Potentiellement ajouter une liste de rectangles de collision si gérés de manière centralisée
} Map;

// Structure pour représenter une zone de collision
typedef struct
{
    SDL_Rect rect;
    char *name;
    char *type; // Optional: "Collision", "Wall", etc.
} CollisionObject;

// Charge une carte TMX et ses ressources associées
Map *loadMap(const char *filePath, SDL_Renderer *renderer);

// Libère la mémoire allouée pour la carte
void freeMap(Map *map);

// Affiche un groupe de calques spécifique de la carte
void Map_afficherGroup(SDL_Renderer *renderer, Map *map, const char *groupName, int offsetX, int offsetY);

// Récupère les objets de collision d'un groupe d'objets spécifique
CollisionObject *Map_getCollisionObjects(Map *map, const char *objectGroupName, int *count);

// Récupère la position de spawn du joueur depuis la carte
bool Map_getPlayerSpawn(Map *map, float *x, float *y);

// Modifie le tile à des coordonnées spécifiques (x, y) dans un calque donné
// Retourne true si la modification a réussi, false sinon
bool Map_setTile(Map *map, const char *layerName, int x, int y, int gid);

// Met à jour les animations des tuiles (si supporté par libtmx et implémenté)
void updateMapAnimations(Map *map);

#endif // MAP_H