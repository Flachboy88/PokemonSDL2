// map.h
#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>
#include <tmx.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float x, y;
} Point;

// Structure pour représenter une zone de collision
typedef struct
{
    SDL_Rect rect;
    char *name;
    char *type; // Optional: "Collision", "Wall", etc.

    Point *polygon_points; // Points du polygone
    int polygon_count;     // Nombre de points
    bool is_polygon;       // true si c'est un polygone, false si c'est un rectangle
} CollisionObject;

// Structure pour stocker les informations de la carte
typedef struct
{
    tmx_map *tmx_map;
    float default_x_spawn;
    float default_y_spawn;
    CollisionObject *collisions;
    int collision_count;

} Map;

// Charge une carte TMX et ses ressources associées
Map *loadMap(const char *filePath, SDL_Renderer *renderer);

// Libère la mémoire allouée pour la carte
void freeMap(Map *map);

// Affiche un groupe de calques spécifique de la carte
// 'current_time' est ajouté pour la gestion des animations par le groupe
void Map_afficherGroup(SDL_Renderer *renderer, Map *map, const char *groupName, int offsetX, int offsetY, uint32_t current_time);

// Récupère les objets de collision d'un groupe d'objets spécifique
CollisionObject *Map_getCollisionObjects(Map *map, const char *objectGroupName, int *count);

// Récupère la position de spawn du joueur depuis la carte
bool Map_getPlayerSpawn(Map *map, float *x, float *y);

// Modifie le tile à des coordonnées spécifiques (x, y) dans un calque donné
// Retourne true si la modification a réussi, false sinon
bool Map_setTile(Map *map, const char *layerName, int x, int y, int gid);

// Initialise les informations d'animation pour toutes les tuiles animées de la carte
// Cette fonction est maintenant déclarée ici car elle est appelée depuis main.c
void Map_initAnimations(Map *map);

// Debug
void DeBugMap(Map *map);
void Map_drawCollisions(SDL_Renderer *renderer, Map *map);

#endif // MAP_H