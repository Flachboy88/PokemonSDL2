#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

// Structure pour une frame d'animation
typedef struct {
    int x, y;           // Position dans la spritesheet
    int width, height;  // Taille de la frame
    int duration;       // Durée en millisecondes
} Frame;

// Structure pour une animation
typedef struct {
    char *name;         // Nom de l'animation
    Frame *frames;      // Tableau des frames
    int frame_count;    // Nombre de frames
    bool loop;          // Animation en boucle
} Animation;

// Structure principale du sprite
typedef struct {
    SDL_Texture *texture;       // Texture de la spritesheet
    int sheet_width, sheet_height; // Taille de la spritesheet
    int frame_width, frame_height; // Taille d'une frame
    int columns, rows;          // Nombre de colonnes/lignes
    
    Animation *animations;      // Tableau des animations
    int animation_count;        // Nombre d'animations
    
    // État actuel
    int current_animation;      // Index de l'animation courante
    int current_frame;          // Frame courante dans l'animation
    Uint32 last_frame_time;     // Temps de la dernière frame
    bool playing;               // Animation en cours
    bool paused;                // Animation en pause
} Sprite;

// Fonctions de création et destruction
Sprite* createSprite(const char *texture_path, int frame_width, int frame_height, SDL_Renderer *renderer);
Sprite* createSpriteWithColumns(const char *texture_path, int columns, int rows, int frame_width, int frame_height, SDL_Renderer *renderer);
void freeSprite(Sprite *sprite);

// Fonctions d'animation
void addAnimation(Sprite *sprite, const char *name, int *frame_indices, int frame_count, int frame_duration, bool loop);
void addSimpleAnimation(Sprite *sprite, const char *name, int start_frame, int end_frame, int frame_duration, bool loop);
bool playAnimation(Sprite *sprite, const char *name);
void pauseAnimation(Sprite *sprite);
void resumeAnimation(Sprite *sprite);
void resetAnimation(Sprite *sprite);
void stopAnimation(Sprite *sprite);

// Fonctions de rendu
void updateSprite(Sprite *sprite);
void renderSprite(Sprite *sprite, SDL_Renderer *renderer, int x, int y);
void renderSpriteScaled(Sprite *sprite, SDL_Renderer *renderer, int x, int y, int width, int height);
void renderSpriteFlipped(Sprite *sprite, SDL_Renderer *renderer, int x, int y, SDL_RendererFlip flip);

// Fonctions utilitaires
bool isAnimationPlaying(Sprite *sprite);
const char* getCurrentAnimationName(Sprite *sprite);
int getCurrentFrame(Sprite *sprite);

#endif