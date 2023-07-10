#include "config.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL_Init: %s\n", SDL_GetError());
    return -1;
  }

  // Create SDL window
  SDL_Window *window = SDL_CreateWindow(
      "Template", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, 0);
  if (!window) {
    printf("SDL_CreateWindow: %s\n", SDL_GetError());
    return -1;
  }

  // Create SDL renderer
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

  if (!renderer) {
    printf("SDL_CreateRenderer: %s\n", SDL_GetError());
    return -1;
  }

  if (TTF_Init()){
    printf("TTF_Init\n");
        return -1;
  }

  int mouse_x = 0;
  int mouse_y = 0;

// Create a text label
    SDL_Color Black = {0, 0, 0, SDL_ALPHA_OPAQUE};
    char label_text[] = "Just a very long text";
    TTF_Font *Inter = TTF_OpenFont("Inter-Regular.ttf", 16);
    SDL_Surface *label_surface =
        TTF_RenderText_Solid(Inter, label_text, Black);
    SDL_Texture *label_texture =
        SDL_CreateTextureFromSurface(renderer, label_surface);
        
  // Begin main loop
  SDL_bool done = SDL_FALSE;
  while (!done) {
    SDL_Event event;
    if (SDL_WaitEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN:
        printf("Key press\n");
        break;
      case SDL_MOUSEMOTION:
        mouse_x = event.motion.x;
        mouse_y = event.motion.y;
        printf("Mouse motion %d, %d\n", mouse_x, mouse_y);
        // Flush event queue to only use one event
        // Otherwise renderer laggs behind while emptying event queue
        SDL_FlushEvent(SDL_MOUSEMOTION);
        break;
      case SDL_QUIT:
        done = SDL_TRUE;
        break;
      }
    }

    // Put rendering code here

    // Clear back buffer
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    if (SDL_RenderClear(renderer)) {
      printf("SDL_RenderClear: %s\n", SDL_GetError());
      return -1;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_Rect testrect = {mouse_x - 16, mouse_y - 16, 32, 32};
    SDL_RenderDrawRect(renderer, &testrect);

    int text_w, text_h;
    TTF_SizeText(Inter, label_text, &text_w, &text_h);
    SDL_Rect text_rect = {mouse_x - 16, mouse_y - 16, text_w, text_h};
    SDL_RenderCopy(renderer, label_texture, NULL, &text_rect);

    // Draw back buffer to screen
    SDL_RenderPresent(renderer);

    // Throttle frame rate to 60fps
    // SDL_Delay(1000 / 60);
  }
    if (label_surface) {
      SDL_FreeSurface(label_surface);
    }
    if (label_texture) {
      SDL_DestroyTexture(label_texture);
    }

  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }

  SDL_Quit();

  return 0;
}
