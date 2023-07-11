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
