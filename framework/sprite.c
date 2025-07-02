#include "sprite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Sprite* createSprite(const char *texture_path, int frame_width, int frame_height, SDL_Renderer *renderer) {
    SDL_Surface *surface = IMG_Load(texture_path);
    if (!surface) {
        fprintf(stderr, "Erreur chargement sprite: %s\n", texture_path);
        return NULL;
    }
    
    Sprite *sprite = calloc(1, sizeof(Sprite));
    sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);
    sprite->sheet_width = surface->w;
    sprite->sheet_height = surface->h;
    sprite->frame_width = frame_width;
    sprite->frame_height = frame_height;
    sprite->columns = sprite->sheet_width / frame_width;
    sprite->rows = sprite->sheet_height / frame_height;
    sprite->current_animation = -1;
    sprite->playing = false;
    sprite->paused = false;
    
    SDL_FreeSurface(surface);
    return sprite;
}


Sprite* createSpriteWithColumns(const char *texture_path, int columns, int rows, int frame_width, int frame_height, SDL_Renderer *renderer) {
    SDL_Surface *surface = IMG_Load(texture_path);
    if (!surface) {
        fprintf(stderr, "Erreur chargement sprite: %s\n", texture_path);
        return NULL;
    }
    
    Sprite *sprite = calloc(1, sizeof(Sprite));
    sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);
    sprite->sheet_width = surface->w;
    sprite->sheet_height = surface->h;
    sprite->columns = columns;
    sprite->rows = rows;
    sprite->frame_width = frame_width; 
    sprite->frame_height = frame_height;  
    sprite->current_animation = -1;
    sprite->playing = false;
    sprite->paused = false;
    
    SDL_FreeSurface(surface);
    return sprite;
}

void freeSprite(Sprite *sprite) {
    if (!sprite) return;
    
    if (sprite->texture) {
        SDL_DestroyTexture(sprite->texture);
    }
    
    for (int i = 0; i < sprite->animation_count; i++) {
        free(sprite->animations[i].name);
        free(sprite->animations[i].frames);
    }
    free(sprite->animations);
    free(sprite);
}

void addAnimation(Sprite *sprite, const char *name, int *frame_indices, int frame_count, int frame_duration, bool loop) {
    sprite->animations = realloc(sprite->animations, (sprite->animation_count + 1) * sizeof(Animation));
    Animation *anim = &sprite->animations[sprite->animation_count];
    
    anim->name = strdup(name);
    anim->frames = calloc(frame_count, sizeof(Frame));
    anim->frame_count = frame_count;
    anim->loop = loop;
    
    for (int i = 0; i < frame_count; i++) {
        int frame_index = frame_indices[i];
        anim->frames[i].x = (frame_index % sprite->columns) * sprite->frame_width;
        anim->frames[i].y = (frame_index / sprite->columns) * sprite->frame_height;
        anim->frames[i].width = sprite->frame_width;
        anim->frames[i].height = sprite->frame_height;
        anim->frames[i].duration = frame_duration;
    }
    
    sprite->animation_count++;
}

void addSimpleAnimation(Sprite *sprite, const char *name, int start_frame, int end_frame, int frame_duration, bool loop) {
    int frame_count = (end_frame - start_frame) + 1;
    int *frame_indices = malloc(frame_count * sizeof(int));
    
    for (int i = 0; i < frame_count; i++) {
        frame_indices[i] = start_frame + i;
    }
    
    addAnimation(sprite, name, frame_indices, frame_count, frame_duration, loop);
    free(frame_indices);
}

bool playAnimation(Sprite *sprite, const char *name) {
    for (int i = 0; i < sprite->animation_count; i++) {
        if (strcmp(sprite->animations[i].name, name) == 0) {
            sprite->current_animation = i;
            sprite->current_frame = 0;
            sprite->last_frame_time = SDL_GetTicks();
            sprite->playing = true;
            sprite->paused = false;
            return true;
        }
    }
    return false;
}

void pauseAnimation(Sprite *sprite) {
    sprite->paused = true;
}

void resumeAnimation(Sprite *sprite) {
    sprite->paused = false;
    sprite->last_frame_time = SDL_GetTicks();
}

void resetAnimation(Sprite *sprite) {
    sprite->current_frame = 0;
    sprite->last_frame_time = SDL_GetTicks();
}

void stopAnimation(Sprite *sprite) {
    sprite->playing = false;
    sprite->paused = false;
    sprite->current_frame = 0;
}

void updateSprite(Sprite *sprite) {
    if (!sprite->playing || sprite->paused || sprite->current_animation < 0) return;
    
    Animation *anim = &sprite->animations[sprite->current_animation];
    Uint32 current_time = SDL_GetTicks();
    
    if (current_time - sprite->last_frame_time >= anim->frames[sprite->current_frame].duration) {
        sprite->current_frame++;
        
        if (sprite->current_frame >= anim->frame_count) {
            if (anim->loop) {
                sprite->current_frame = 0;
            } else {
                sprite->current_frame = anim->frame_count - 1;
                sprite->playing = false;
            }
        }
        
        sprite->last_frame_time = current_time;
    }
}

void renderSprite(Sprite *sprite, SDL_Renderer *renderer, int x, int y) {
    if (!sprite->texture || sprite->current_animation < 0) return;
    
    Animation *anim = &sprite->animations[sprite->current_animation];
    Frame *frame = &anim->frames[sprite->current_frame];
    
    SDL_Rect src_rect = {frame->x, frame->y, frame->width, frame->height};
    SDL_Rect dst_rect = {x, y, frame->width, frame->height};
    
    SDL_RenderCopy(renderer, sprite->texture, &src_rect, &dst_rect);
}

void renderSpriteScaled(Sprite *sprite, SDL_Renderer *renderer, int x, int y, int width, int height) {
    if (!sprite->texture || sprite->current_animation < 0) return;
    
    Animation *anim = &sprite->animations[sprite->current_animation];
    Frame *frame = &anim->frames[sprite->current_frame];
    
    SDL_Rect src_rect = {frame->x, frame->y, frame->width, frame->height};
    SDL_Rect dst_rect = {x, y, width, height};
    
    SDL_RenderCopy(renderer, sprite->texture, &src_rect, &dst_rect);
}

void renderSpriteFlipped(Sprite *sprite, SDL_Renderer *renderer, int x, int y, SDL_RendererFlip flip) {
    if (!sprite->texture || sprite->current_animation < 0) return;
    
    Animation *anim = &sprite->animations[sprite->current_animation];
    Frame *frame = &anim->frames[sprite->current_frame];
    
    SDL_Rect src_rect = {frame->x, frame->y, frame->width, frame->height};
    SDL_Rect dst_rect = {x, y, frame->width, frame->height};
    
    SDL_RenderCopyEx(renderer, sprite->texture, &src_rect, &dst_rect, 0, NULL, flip);
}

bool isAnimationPlaying(Sprite *sprite) {
    return sprite->playing && !sprite->paused;
}

const char* getCurrentAnimationName(Sprite *sprite) {
    if (sprite->current_animation >= 0) {
        return sprite->animations[sprite->current_animation].name;
    }
    return NULL;
}

int getCurrentFrame(Sprite *sprite) {
    return sprite->current_frame;
}