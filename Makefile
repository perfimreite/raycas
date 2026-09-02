CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic
DEPENDECIES = sdl_main.c game.c utils.c vector.c constants.h game.h base.h sdl_main.h

all: sdl_main sdlmain_hd

sdl_main:$(DEPENDECIES)
	@$(CC) $(CFLAGS) sdl_main.c game.c utils.c vector.c -I/usr/include/SDL2 -Ithirdparty -lSDL2 -lSDL2main -lSDL2_ttf -lm -o sdl_main -g

sdl_main_hd: $(DEPENDECIES)
	@$(CC) $(CFLAGS) -DHIGH_RESOLUTION sdl_main.c game.c utils.c vector.c -I/usr/include/SDL2 -Ithirdparty -lSDL2 -lSDL2main -lSDL2_ttf -lm -o sdl_main_hd -g

clean:
	rm -rf sdl_main
	rm -rf sdl_main_hd
	rm -rf *.o

