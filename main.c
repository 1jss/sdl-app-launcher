#include "config.h"
#include <SDL2/SDL.h>

int main(int argc, char *argv[]) {
  // initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL_Init: %s\n", SDL_GetError());
    return -1;
  }

  // create the window
  SDL_Window *window = SDL_CreateWindow(
      "Template", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, 0);
  if (!window) {
    printf("SDL_CreateWindow: %s\n", SDL_GetError());
    return -1;
  }

  // create a renderer for the window
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

  if (!renderer) {
    printf("SDL_CreateRenderer: %s\n", SDL_GetError());
    return -1;
  }

  // begin main loop
  SDL_bool done = SDL_FALSE;
  int frame_index = 0;

  while (!done) {
    printf("index: %d\n", frame_index);
    frame_index++;

    SDL_Event event;

    if (SDL_WaitEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN:
        printf("Key press\n");
        break;
      case SDL_MOUSEMOTION:
        printf("Mouse motion\n");
        break;
      case SDL_QUIT:
        done = SDL_TRUE;
        break;
      }
    }

    // Put rendering code here
    if (SDL_RenderClear(renderer)) {
      printf("SDL_RenderClear: %s\n", SDL_GetError());
      return -1;
    }

    // flip the display
    SDL_RenderPresent(renderer);

    // throttle the frame rate to 60fps
    SDL_Delay(1000 / 60);
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
