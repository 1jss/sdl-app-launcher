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
} DesktopFile;

typedef struct Launcher {
  char exec[128];
  SDL_Texture *icon_texture;
  SDL_Rect icon_rect;
  SDL_Texture *label_texture;
  SDL_Rect label_rect;
  struct Launcher *next;
} Launcher;

Launcher *addLauncher(struct Launcher *head, char exec[128],
                      SDL_Texture *icon_texture, SDL_Rect icon_rect,
                      SDL_Texture *label_texture, SDL_Rect label_rect) {
  Launcher *newLauncher = (Launcher *)malloc(sizeof(Launcher));
  if (newLauncher == NULL) {
    printf("Failed to allocate memory.\n");
    return head;
  }
  strcpy(newLauncher->exec, exec);
  newLauncher->icon_texture = icon_texture;
  newLauncher->icon_rect = icon_rect;
  newLauncher->label_texture = label_texture;
  newLauncher->label_rect = label_rect;
  newLauncher->next = NULL;

  // Handle first launcher
  if (head == NULL) {
    return newLauncher;
  }

  // Put new launcher last in the linked list
  Launcher *current = head;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = newLauncher;
  return head;
}

void freeLaunchers(Launcher *head) {
  Launcher *current = head;
  while (current != NULL) {
    Launcher *next = current->next;
    free(current);
    current = next;
  }
}

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  const SDL_Color WHITE = {255, 255, 255, SDL_ALPHA_OPAQUE};
  const char applications_url[] = "/usr/share/applications/";
  DIR *directory;
  struct dirent *dir_entry;
  FILE *desktop_file;
  char buffer[BUFSIZ];
  GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
  Launcher *launcherList = NULL;
  DesktopFile desktop_files[128];
  for (int i = 0; i < 128; i++) {
    strcpy(desktop_files[i].name, "");
    strcpy(desktop_files[i].icon, "");
    strcpy(desktop_files[i].exec, "");
    desktop_files[i].terminal = false;
    desktop_files[i].no_display = false;
  };
  int mouse_x = 0;
  int mouse_y = 0;
  int scroll_y = 0;
  int col = 0;
  int row = 0;
  const int colstart = 48;
  const int colwidth = 96;
  const int rowstart = 32;
  const int rowheight = 112;

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
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

  // Initialize font
  if (TTF_Init()) {
    printf("TTF_Init\n");
    return -1;
  }
  TTF_Font *Inter = TTF_OpenFont("Inter-Regular.ttf", 16);

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

    // Construct full urls
    char file_url[strlen(applications_url) + strlen(dir_entry->d_name)];
    strcpy(file_url, applications_url);
    strcat(file_url, dir_entry->d_name);

    // Open file
    desktop_file = fopen(file_url, "r");
    if (desktop_file == NULL) {
      printf("Failed to open entry file.\n");
      return 1;
    }

    // Parse file line by line
    while (fgets(buffer, BUFSIZ, desktop_file) != NULL) {
      if (strlen(desktop_files[i].name) == 0 &&
          strncmp("Name=", buffer, strlen("Name=")) == 0) {
        // Find equal sign position
        char *equal_sign_pos = strchr(buffer, '=');
        if (equal_sign_pos != NULL) {
          // Start string after equal sign
          char *app_name = equal_sign_pos + 1;
          // Remove newline
          app_name[strcspn(app_name, "\n")] = '\0';
          strcpy(desktop_files[i].name, app_name);
        }
      }
      if (strlen(desktop_files[i].icon) == 0 &&
          strncmp("Icon=", buffer, strlen("Icon=")) == 0) {
        char *equal_sign_pos = strchr(buffer, '=');
        if (equal_sign_pos != NULL) {
          // Start string after equal sign
          char *icon_name = equal_sign_pos + 1;
          // Remove newline
          icon_name[strcspn(icon_name, "\n")] = '\0';

          if (icon_name[0] == '/') {
            strcpy(desktop_files[i].icon, icon_name);
          } else {
            GtkIconInfo *icon_info =
                gtk_icon_theme_lookup_icon(icon_theme, icon_name, 64, 0);
            if (icon_info != NULL) {
              const gchar *icon_filename =
                  gtk_icon_info_get_filename(icon_info);
              if (icon_filename != NULL) {
                strcpy(desktop_files[i].icon, icon_filename);
              }
              g_object_unref(icon_info);
            }
          }
        }
      }
      if (strlen(desktop_files[i].exec) == 0 &&
          strncmp("Exec=", buffer, strlen("Exec=")) == 0) {
        char *equal_sign_pos = strchr(buffer, '=');
        if (equal_sign_pos != NULL) {
          char *app_exec = equal_sign_pos + 1;
          // End string before newline
          app_exec[strcspn(app_exec, "\n")] = '\0';
          // End string at first space
          app_exec[strcspn(app_exec, " ")] = '\0';
          strcpy(desktop_files[i].exec, app_exec);
        }
      }
      if (strncmp("Terminal=true", buffer, strlen("Terminal=true")) == 0) {
        desktop_files[i].terminal = true;
      }
      if (strncmp("NoDisplay=true", buffer, strlen("NoDisplay=true")) == 0) {
        desktop_files[i].no_display = true;
      }
    }
    fclose(desktop_file);

    if (strlen(desktop_files[i].name) != 0 &&
        desktop_files[i].no_display == false &&
        strlen(desktop_files[i].icon) != 0) {

      SDL_Texture *icon_texture =
          IMG_LoadTexture(renderer, desktop_files[i].icon);
      SDL_Rect icon_rect = (SDL_Rect){colstart + col * colwidth,
                                      rowstart + row * rowheight, 64, 64};

      SDL_Surface *label_surface =
          TTF_RenderUTF8_Blended(Inter, desktop_files[i].name, WHITE);
      SDL_Texture *label_texture =
          SDL_CreateTextureFromSurface(renderer, label_surface);
      if (label_surface) {
        SDL_FreeSurface(label_surface);
      }

      int text_w, text_h;
      TTF_SizeText(Inter, desktop_files[i].name, &text_w, &text_h);
      SDL_Rect label_rect =
          (SDL_Rect){colstart + col * colwidth, rowstart + row * rowheight + 72,
                     text_w, text_h};

      launcherList =
          addLauncher(launcherList, desktop_files[i].exec, icon_texture,
                      icon_rect, label_texture, label_rect);

      if (col == 5) {
        col = 0;
        row++;
      } else {
        col++;
      }
    }
    i++;
  };

  // Begin main loop
  SDL_bool done = SDL_FALSE;
  bool found;
  // int index;
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
        // index = 0;
        found = false;
        Launcher *launcherHead = launcherList;
        while (found == false && launcherHead != NULL) {
          if (mouse_x > launcherHead->icon_rect.x &&
              mouse_x < launcherHead->icon_rect.x + launcherHead->icon_rect.w &&
              mouse_y > launcherHead->icon_rect.y &&
              mouse_y < launcherHead->icon_rect.y + launcherHead->icon_rect.h) {
            found = true;
            // Launch new process
            if (fork() == 0) {
              system(launcherHead->exec);
              return 1;
            }
          }
          launcherHead = launcherHead->next;
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
    Launcher *current = launcherList;
    while (current != NULL) {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
      SDL_RenderCopy(renderer, current->icon_texture, NULL,
                     &current->icon_rect);
      SDL_RenderCopy(renderer, current->label_texture, NULL,
                     &current->label_rect);

      int xpos = current->label_rect.x + 64;
      int ypos = current->label_rect.y;
      // Gradient mask for label
      for (int j = 0; j < 16; j++) {
        SDL_SetRenderDrawColor(renderer, 36, 36, 36, j * 16);
        SDL_RenderDrawLine(renderer, xpos + j, ypos, xpos + j, ypos + 32);
      }
      SDL_SetRenderDrawColor(renderer, 36, 36, 36, SDL_ALPHA_OPAQUE);
      SDL_Rect maskrect = {current->label_rect.x + 80, current->label_rect.y,
                           current->label_rect.w, current->label_rect.h};
      SDL_RenderFillRect(renderer, &maskrect);
      current = current->next;
    }

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

  freeLaunchers(launcherList);

  TTF_CloseFont(Inter);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
