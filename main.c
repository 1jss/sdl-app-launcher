#include "config.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
//#include <regex.h>
//#include <unistd.h>

int main(int argc, char *argv[]) {

  /*
  Plan:
  - Create data structure for .desktop files
  - Compile regex for name, icon, exe, termanal and visability
  - List all files in applications folder
  - Loop through all files
  - Put regexed content in data structure
  - Draw content of data structure to screen
  - Create rectangles of mouse targets
  - Check for overlap on click
  - Launch new process with app exe command


  // Launch new process
  if (fork() == 0) {
    system("my_app");
    return 1;
  }
  return 0;
  */

  /*
    regex_t regex;
    int reti;
    char msgbuf[100];

    // Compile regular expression
    reti = regcomp(&regex, "^a[[:alnum:]]", 0);

    regcomp(&regex_name, "Name=(.*),0);
    regcomp(&regex_icon,"Icon=([^[:space:]]*).*",0);
    regcomp(&regex_exec, "Exec=(.*)",0);
    regcomp(&regex_terminal, "Terminal=true",0);
    regcomp(&regex_nodisplay,NoDisplay=([^[:space:]]*).*,0);

    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      exit(1);
    }

    // Execute regular expression
    reti = regexec(&regex, "abc", 0, NULL, 0);
    if (!reti) {
      puts("Match");
    }
    else if (reti == REG_NOMATCH) {
      puts("No match");
    }
    else {
      regerror(reti, &regex, msgbuf, sizeof(msgbuf));
      fprintf(stderr, "Regex match failed: %s\n", msgbuf);
      exit(1);
    }

  // Free memory allocated to the pattern buffer by regcomp()
     regfree(&regex);


  */

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

  if (TTF_Init()) {
    printf("TTF_Init\n");
    return -1;
  }

  int mouse_x = 0;
  int mouse_y = 0;

  // Create a text label
  const SDL_Color BLACK = {0, 0, 0, SDL_ALPHA_OPAQUE};
  const SDL_Color WHITE = {255, 255, 255, SDL_ALPHA_OPAQUE};

  char label_text[] = "Just a very long text";
  TTF_Font *Inter = TTF_OpenFont("Inter-Regular.ttf", 16);
  SDL_Surface *label_surface = TTF_RenderUTF8_Blended(Inter, label_text, BLACK);
  SDL_Texture *label_texture =
      SDL_CreateTextureFromSurface(renderer, label_surface);

  SDL_Surface *label_surface_shaded =
      TTF_RenderUTF8_Shaded(Inter, label_text, BLACK, WHITE);
  SDL_Texture *label_texture_shaded =
      SDL_CreateTextureFromSurface(renderer, label_surface_shaded);

  SDL_Texture *icon_application_texture =
      IMG_LoadTexture(renderer, "icons/application-x-executable.bmp");
  SDL_Texture *svg_icon_texture =
      IMG_LoadTexture(renderer, "./SVG/display.svg");

  // Begin main loop
  SDL_bool done = SDL_FALSE;
  while (!done) {
    SDL_Event event;
    if (SDL_WaitEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN:
        // printf("Key press\n");
        break;
      case SDL_MOUSEMOTION:
        mouse_x = event.motion.x;
        mouse_y = event.motion.y;
        // printf("Mouse motion %d, %d\n", mouse_x, mouse_y);
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

    // Draw SVG to screen
    SDL_Rect svg_icon_rect = {10, 10, 128, 128};
    SDL_RenderCopy(renderer, svg_icon_texture, NULL, &svg_icon_rect);

    // Draw text to screen
    int text_w, text_h;
    TTF_SizeText(Inter, label_text, &text_w, &text_h);
    SDL_Rect text_rect = {mouse_x - text_w - 16, mouse_y - text_h - 8, text_w,
                          text_h};
    SDL_RenderCopy(renderer, label_texture, NULL, &text_rect);

    SDL_Rect text_rect_shaded = {mouse_x - text_w - 16, mouse_y, text_w,
                                 text_h};
    SDL_RenderCopy(renderer, label_texture_shaded, NULL, &text_rect_shaded);

    // Draw rect to screen
    SDL_Rect testrect = {mouse_x - text_w - 32, mouse_y - text_h - 16,
                         text_w + 32, text_h + 16};
    SDL_RenderDrawRect(renderer, &testrect);

    // Draw icon to screen
    SDL_Rect icon_rect = {mouse_x, mouse_y - 16, 16, 16};
    SDL_RenderCopy(renderer, icon_application_texture, NULL, &icon_rect);

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

  if (label_surface_shaded) {
    SDL_FreeSurface(label_surface_shaded);
  }
  if (label_texture_shaded) {
    SDL_DestroyTexture(label_texture_shaded);
  }

  if (icon_application_texture) {
    SDL_DestroyTexture(icon_application_texture);
  }

  if (svg_icon_texture) {
    SDL_DestroyTexture(svg_icon_texture);
  }

  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }

  TTF_CloseFont(Inter);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
