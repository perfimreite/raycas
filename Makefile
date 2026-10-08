CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic

all: sdl2_main sdl2_main_hd sdl3_main sdl3_main_hd

# --- SDL 2 ---

SDL2_DEPENDECIES = sdl2_main.c sdl2_main.h game.c game.h utils.c vector.c base.h
sdl2_main:$(SDL2_DEPENDECIES)
	$(CC) $(CFLAGS) sdl2_main.c game.c utils.c vector.c -I/usr/include/SDL2 -Ithirdparty -lSDL2 -lSDL2main -lSDL2_ttf -lm -o sdl2_main

sdl2_main_hd: $(SDL2_DEPENDECIES)
	$(CC) $(CFLAGS) -DHIGH_RESOLUTION sdl2_main.c game.c utils.c vector.c -I/usr/include/SDL2 -Ithirdparty -lSDL2 -lSDL2main -lSDL2_ttf -lm -o sdl2_main_hd

# --- SDL 3 ---

SDL2_DEPENDECIES = sdl3_main.c sdl3_main.h game.c game.h utils.c vector.c base.h
sdl3_main:$(SDL3_DEPENDECIES)
	$(CC) $(CFLAGS) sdl3_main.c game.c utils.c vector.c -I/usr/include/SDL3 -Ithirdparty -lSDL3 -lSDL3_ttf -lm -o sdl3_main

sdl3_main_hd: $(SDL3_DEPENDECIES)
	$(CC) $(CFLAGS) -DHIGH_RESOLUTION sdl3_main.c game.c utils.c vector.c -I/usr/include/SDL3 -Ithirdparty -lSDL3 -lSDL3_ttf -lm -o sdl3_main_hd

# --- clean ---
clean:
	rm -rf sdl2_main
	rm -rf sdl2_main_hd
	rm -rf sdl3_main
	rm -rf sdl3_main_hd
	rm -rf *.o

