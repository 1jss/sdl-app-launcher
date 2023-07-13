TITLE=main
CFLAGS=`sdl2-config sdl2 --cflags --libs` ${LIBS} -o $(TITLE)
LIBS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image

$(TITLE): *.c *.h
	$(CC) $(TITLE).c -Wall -std=c99 $(CFLAGS) $(LIBS)
