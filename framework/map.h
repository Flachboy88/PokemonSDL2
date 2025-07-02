#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

// Structure pour un tileset
typedef struct {
    int firstgid;           // Premier ID du tileset
    int tilecount;          // Nombre de tiles
    int tilewidth;          // Largeur d'une tile
    int tileheight;         // Hauteur d'une tile
    int columns;            // Nombre de colonnes
    char *name;             // Nom du tileset
    char *image_path;       // Chemin vers l'image
    SDL_Texture *texture;   // Texture SDL chargée
} Tileset;

// Structure pour un layer de tiles
typedef struct {
    int id;
    char *name;
    int width;
    int height;
    int *data;              // Données des tiles (tableau 1D)
    bool visible;
    float opacity;
    int x, y;               // Offset du layer
} TileLayer;

// Structure pour un objet
typedef struct {
    int id;
    char *name;
    char *type;
    float x, y;
    float width, height;
    bool point;             // Si c'est un point
    float rotation;
    bool visible;
} MapObject;

// Structure pour un layer d'objets
typedef struct {
    int id;
    char *name;
    MapObject *objects;
    int object_count;
    bool visible;
    float opacity;
    int x, y;
} ObjectLayer;

// Structure pour un groupe de layers
typedef struct {
    int id;
    char *name;
    TileLayer *tile_layers;
    int tile_layer_count;
    ObjectLayer *object_layers;
    int object_layer_count;
    bool visible;
    float opacity;
    int x, y;
} LayerGroup;

// Structure principale de la map
typedef struct {
    int width;              // Largeur en tiles
    int height;             // Hauteur en tiles
    int tilewidth;          // Largeur d'une tile
    int tileheight;         // Hauteur d'une tile
    
    Tileset *tilesets;      // Tableaux des tilesets
    int tileset_count;
    
    LayerGroup *layer_groups;
    int layer_group_count;
    
    TileLayer *tile_layers; // Layers directs (non groupés)
    int tile_layer_count;
    
    ObjectLayer *object_layers; // Object layers directs
    int object_layer_count;
} Map;

// Fonctions principales
Map* loadMap(const char *filename, SDL_Renderer *renderer);
void renderMap(Map *map, SDL_Renderer *renderer);
void renderMapToLayer(Map *map, SDL_Renderer *renderer, const char *layer_name);
void renderMapBeforeLayer(Map *map, SDL_Renderer *renderer, const char *layer_name);
void renderMapAfterLayer(Map *map, SDL_Renderer *renderer, const char *layer_name);
void freeMap(Map *map);

// Fonctions utilitaires
Tileset* getTilesetForGID(Map *map, int gid);
MapObject* getObjectByName(Map *map, const char *name);
MapObject* getObjectsByType(Map *map, const char *type, int *count);

#endif