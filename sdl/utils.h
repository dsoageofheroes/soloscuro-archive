#ifndef SOL_SDL_UTILS_H
#define SOL_SDL_UTILS_H

#include <SDL2/SDL.h>

// Utility function
SDL_Texture* create_texture(const uint32_t gff_file,
        const uint32_t type, const uint32_t id, const uint32_t frame_id,
        const uint32_t palette_id, SDL_Rect *loc);

#endif
