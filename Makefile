CC = gcc
CFLAGS = -Wall -Wextra -std=c11

run: build/main
	build/main

build/main: main.c
	$(CC) $(CFLAGS) main.c vector.c -I/usr/include/SDL2 -lSDL2 -lSDL2main -lSDL2_ttf -lm -o build/main -g

clean:
	rm -rf build/main

