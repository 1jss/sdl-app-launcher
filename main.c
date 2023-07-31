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

typedef struct DesktopFile {
  char name[128];
  char icon[128];
  char exec[128];
  bool terminal;
  bool no_display;
  SDL_Texture *icon_texture;
  SDL_Rect icon_rect;
  SDL_Texture *label_texture;
  SDL_Rect label_rect;

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
              gtk_icon_theme_lookup_icon(icon_theme, icon_name, 64, 0);

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

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

  // Create SDL renderer
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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
  int scroll_y = 0;

  // const SDL_Color BLACK = {0, 0, 0, SDL_ALPHA_OPAQUE};
  const SDL_Color WHITE = {255, 255, 255, SDL_ALPHA_OPAQUE};

  // Init font
  TTF_Font *Inter = TTF_OpenFont("Inter-Regular.ttf", 16);

  // Create textures for icons
  int col = 0;
  int row = 0;
  const int colstart = 48;
  const int colwidth = 96;
  const int rowstart = 32;
  const int rowheight = 112;
  for (int i = 0; i < 128; i++) {
    if (strlen(desktop_files[i].name) != 0 &&
        desktop_files[i].no_display == false &&
        strlen(desktop_files[i].icon) != 0) {
      // printf("Name: %s\n", desktop_files[i].name);
      // printf("Icon: %s\n", desktop_files[i].icon);
      // printf("Exec: %s\n", desktop_files[i].exec);
      // printf("Terminal: %s\n", desktop_files[i].terminal ? "true" : "false");
      // printf("NoDisplay: %s\n", desktop_files[i].no_display ? "true" :
      // "false");
      desktop_files[i].icon_texture =
          IMG_LoadTexture(renderer, desktop_files[i].icon);
      desktop_files[i].icon_rect = (SDL_Rect){
          colstart + col * colwidth, rowstart + row * rowheight, 64, 64};

      SDL_Surface *label_surface =
          TTF_RenderUTF8_Blended(Inter, desktop_files[i].name, WHITE);
      desktop_files[i].label_texture =
          SDL_CreateTextureFromSurface(renderer, label_surface);
      if (label_surface) {
        SDL_FreeSurface(label_surface);
      }

      int text_w, text_h;
      TTF_SizeText(Inter, desktop_files[i].name, &text_w, &text_h);
      desktop_files[i].label_rect =
          (SDL_Rect){colstart + col * colwidth, rowstart + row * rowheight + 72,
                     text_w, text_h};
      if (col == 5) {
        col = 0;
        row++;
      } else {
        col++;
      }
    }
  };

  // Begin main loop
  SDL_bool done = SDL_FALSE;
  bool found;
  int index;
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
      case SDL_MOUSEWHEEL:
        // scroll up
        if (event.wheel.y > 0) {
          scroll_y += event.wheel.y;
          printf("scroll: %d\n", scroll_y);
        }
        // scroll down
        else if (event.wheel.y < 0) {
          scroll_y += event.wheel.y;
          printf("scroll: %d\n", scroll_y);
        }
        SDL_FlushEvent(SDL_MOUSEWHEEL);
        break;
      case SDL_MOUSEBUTTONDOWN:
        SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
        index = 0;
        found = false;
        while (found == false && index < 128) {
          printf("index: %d\n", index);
          printf("found: %s\n", found ? "true" : "false");
          printf("mouse_x: %d\n", mouse_x);
          if (strlen(desktop_files[index].name) != 0 &&
              desktop_files[index].no_display == false &&
              strlen(desktop_files[index].icon) != 0) {
            printf("rect_x%d\n", desktop_files[index].icon_rect.x);
            printf("rect_w%d\n", desktop_files[index].icon_rect.w);

            if (mouse_x > desktop_files[index].icon_rect.x &&
                mouse_x < desktop_files[index].icon_rect.x +
                              desktop_files[index].icon_rect.w &&
                mouse_y > desktop_files[index].icon_rect.y &&
                mouse_y < desktop_files[index].icon_rect.y +
                              desktop_files[index].icon_rect.h) {
              found = true;
              // Launch new process
              if (fork() == 0) {
                system(desktop_files[index].exec);
                return 1;
              }
            }
          }
          index++;
        }
        break;
      case SDL_QUIT:
        done = SDL_TRUE;
        break;
      }
    }

    // Clear back buffer
    SDL_SetRenderDrawColor(renderer, 36, 36, 36, SDL_ALPHA_OPAQUE);
    if (SDL_RenderClear(renderer)) {
      printf("SDL_RenderClear: %s\n", SDL_GetError());
      return -1;
    }

    // Draw icons and labels to screen
    for (int i = 0; i < 128; i++) {
      if (strlen(desktop_files[i].name) != 0 &&
          desktop_files[i].no_display != true &&
          strlen(desktop_files[i].icon) != 0) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderCopy(renderer, desktop_files[i].icon_texture, NULL,
                       &desktop_files[i].icon_rect);
        SDL_RenderCopy(renderer, desktop_files[i].label_texture, NULL,
                       &desktop_files[i].label_rect);

        int ypos = desktop_files[i].label_rect.y;
        int xpos = desktop_files[i].label_rect.x + 64;
        // Gradient mask for label
        for (int j = 0; j < 16; j++) {
          SDL_SetRenderDrawColor(renderer, 36, 36, 36, j * 16);
          SDL_RenderDrawLine(renderer, xpos + j, ypos, xpos + j, ypos + 32);
        }
        SDL_SetRenderDrawColor(renderer, 36, 36, 36, SDL_ALPHA_OPAQUE);
        SDL_Rect maskrect = {
            desktop_files[i].label_rect.x + 80, desktop_files[i].label_rect.y,
            desktop_files[i].label_rect.w, desktop_files[i].label_rect.h};
        SDL_RenderFillRect(renderer, &maskrect);
      }
    };

    // Draw back buffer to screen
    SDL_RenderPresent(renderer);

    // Throttle frame rate to 60fps
    // SDL_Delay(1000 / 60);
  } // End of rendering loop

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
