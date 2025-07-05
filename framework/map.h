// map.h
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

} Map;

// Structure pour représenter une zone de collision
typedef struct
{
    SDL_Rect rect;
    char *name;
    char *type; // Optional: "Collision", "Wall", etc.
} CollisionObject;

typedef struct
{
    uint32_t local_tile_id;     // Local ID of the animated tile within its tileset
    uint32_t tileset_first_gid; // First GID of the tileset this tile belongs to
    tmx_tile *tmx_tile_ptr;     // Pointeur vers la structure tmx_tile pour cette animation
    int current_frame_index;    // Index de la frame actuelle dans l'animation
    uint32_t frame_start_time;  // Temps (en ms) où la frame actuelle a commencé à s'afficher
} AnimatedTileInfo;

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

// Initialise les informations d'animation pour toutes les tuiles animées de la carte
void Map_initAnimations(Map *map);

// Met à jour l'état des animations de la carte en fonction du temps
void Map_updateAnimations(Map *map, uint32_t current_time);

#endif // MAP_H