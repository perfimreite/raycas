CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic

all: main main_hd

main: main.c game.c map.c utils.c vector.c constants.h
	$(CC) $(CFLAGS) main.c game.c map.c utils.c vector.c -I/usr/include/SDL2 -lSDL2 -lSDL2main -lSDL2_ttf -lm -o main -g

main_hd: main.c game.c map.c utils.c vector.c constants.h
	$(CC) $(CFLAGS) -DHIGH_RESOLUTION main.c game.c map.c utils.c vector.c -I/usr/include/SDL2 -lSDL2 -lSDL2main -lSDL2_ttf -lm -o main_hd -g

clean:
	rm -rf main
	rm -rf main_hd
	rm -rf *.o

