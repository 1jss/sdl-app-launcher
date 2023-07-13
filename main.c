#include "config.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <unistd.h>

typedef struct DesktopFile {
  char name[128];
  char icon[128];
  char exec[128];
  bool terminal;
  bool no_display;
} DesktopFile;

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  DesktopFile desktop_files[128];
  for (int i = 0; i < 128; i++) {
    strcpy(desktop_files[i].name, "");
    strcpy(desktop_files[i].icon, "");
    strcpy(desktop_files[i].exec, "");
    desktop_files[i].terminal = false;
    desktop_files[i].no_display = false;
  };

  const char applications_url[] = "/usr/share/applications/";
  DIR *directory;
  struct dirent *dir_entry;
  FILE *desktop_file;
  char buffer[BUFSIZ];
  GtkIconTheme *icon_theme = gtk_icon_theme_get_default();

  if (NULL == (directory = opendir(applications_url))) {
    printf("Failed to open directory.\n");
    return 1;
  }
  int i = 0;
  while ((dir_entry = readdir(directory))) {
    // Skip current and parent directory
    if (!strcmp(dir_entry->d_name, "."))
      continue;
    if (!strcmp(dir_entry->d_name, ".."))
      continue;

    char file_url[strlen(applications_url) + strlen(dir_entry->d_name)];
    strcpy(file_url, applications_url);
    strcat(file_url, dir_entry->d_name);

    desktop_file = fopen(file_url, "r");
    if (desktop_file == NULL) {
      printf("Failed to open entry file.\n");
      return 1;
    }

    // Parse file line by line
    while (fgets(buffer, BUFSIZ, desktop_file) != NULL) {
      if (strlen(desktop_files[i].name) == 0 &&
          strncmp("Name=", buffer, strlen("Name=")) == 0) {
        strtok(buffer, "="); // Throw away "Name="
        char name[128];
        strcpy(name, strtok(NULL, "="));
        name[strcspn(name, "\n")] = '\0';
        strcpy(desktop_files[i].name, name);
      }
      if (strlen(desktop_files[i].icon) == 0 &&
          strncmp("Icon=", buffer, strlen("Icon=")) == 0) {
        strtok(buffer, "="); // Throw away "Icon="
        char icon_name[128];
        strcpy(icon_name, strtok(NULL, "="));
        icon_name[strcspn(icon_name, "\n")] = '\0';

        if (icon_name[0] == '/') {
          strcpy(desktop_files[i].icon, icon_name);
        } else {

          GtkIconInfo *icon_info =
              gtk_icon_theme_lookup_icon(icon_theme, icon_name, 32, 0);

          if (icon_info != NULL) {
            const gchar *icon_filename = gtk_icon_info_get_filename(icon_info);

            if (icon_filename != NULL) {
              printf("Found icon entry!\n");
              printf("%s\n", icon_filename);
              strcpy(desktop_files[i].icon, icon_filename);
            }
            g_object_unref(icon_info);
          }
        }
      }
      if (strlen(desktop_files[i].exec) == 0 &&
          strncmp("Exec=", buffer, strlen("Exec=")) == 0) {
        strtok(buffer, "="); // Throw away "Exec="
        char exec[128];
        strcpy(exec, strtok(NULL, "="));
        exec[strcspn(exec, "\n")] = '\0';
        strcpy(desktop_files[i].exec, exec);
      }
      if (strncmp("Terminal=true", buffer, strlen("Terminal=true")) == 0) {
        desktop_files[i].terminal = true;
      }
      if (strncmp("NoDisplay=true", buffer, strlen("NoDisplay=true")) == 0) {
        desktop_files[i].no_display = true;
      }
    }
    fclose(desktop_file);
    i++;
  }
  for (int i = 0; i < 128; i++) {
    if (strlen(desktop_files[i].name) != 0) {
      printf("Name: %s\n", desktop_files[i].name);
      printf("Icon: %s\n", desktop_files[i].icon);
      printf("Exec: %s\n", desktop_files[i].exec);
      printf("Terminal: %s\n", desktop_files[i].terminal ? "true" : "false");
      printf("NoDisplay: %s\n", desktop_files[i].no_display ? "true" : "false");
    }
  };

  /*

  // Launch new process
  if (fork() == 0) {
    system("my_app");
    return 1;
  }
  return 0;
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
