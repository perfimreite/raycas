CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic

run: build/main
	build/main

build/main: main.c constants.h map.c utils.c vector.c
	$(CC) $(CFLAGS) main.c vector.c map.c utils.c -I/usr/include/SDL2 -lSDL2 -lSDL2main -lSDL2_ttf -lm -o build/main -g

run-high-res: build/main_high_res
	build/main_high_res

build/main_high_res: main.c constants.h map.c utils.c vector.c
	$(CC) $(CFLAGS) -DHIGH_RESOLUTION=1 main.c vector.c map.c utils.c -I/usr/include/SDL2 -lSDL2 -lSDL2main -lSDL2_ttf -lm -o build/main_high_res -g

clean:
	rm -rf build/main
	rm -rf build/main_high_res

