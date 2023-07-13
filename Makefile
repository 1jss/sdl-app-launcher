# Makefile variables
CC = gcc
CFLAGS = -Wall -std=c99
LIBS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image
GTKLIBS = `pkg-config --libs gtk+-3.0`
GTKFLAGS = `pkg-config --cflags gtk+-3.0`

# Target executable
TARGET = main

# Source files
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

# Build rule
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LIBS) $(GTKLIBS) -o $(TARGET)

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) $(GTKFLAGS) -c $< -o $@

# Phony target to clean the build
clean:
	rm -f $(OBJS) $(TARGET)
