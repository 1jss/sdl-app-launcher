# This is my notes from learning C and SDL

Formatting code:
`clang-format -i main.c`

Compiling manually:
`gcc -lSDL2 -lSDL2_ttf -o main main.c`

Text rendering:
1. Foreground and background:
    `TTF_RenderUTF8_Shaded(Inter, label_text, BLACK, WHITE);`
2. Foreground and transparent:
    `TTF_RenderUTF8_Blended(Inter, label_text, BLACK);`

Image rendering
1. Render image to surface and then to texture:
```
  SDL_Surface *icon_application_surface =
      SDL_LoadBMP("icons/application-x-executable.bmp");
  SDL_Texture *icon_application_texture =
      SDL_CreateTextureFromSurface(renderer, icon_application_surface);
```
2. Render image directly to texture:
`SDL_Texture *icon_application_texture =  IMG_LoadTexture(renderer,"icons/application-x-executable.bmp");
`

Plan:
  - [x] Create data structure for .desktop files
  - [x] List all files in applications folder
  - [x] Loop through all files
  - [x] Parse .desktop files for name, icon, exe, termanal and visability
  - [x] Fetch icon URIs from system?
  - [x] Put parsed content in data structure
  - [ ] Draw content of data structure to screen
  - [ ] Create rectangles of mouse targets
  - [ ] Check for overlap on click
  - [ ] Launch new process with app exe command
