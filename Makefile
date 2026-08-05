CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic
DEPENDECIES = main.c game.c map.c utils.c vector.c constants.h game.h base.h

all: main main_hd

main:$(DEPENDECIES)
	$(CC) $(CFLAGS) main.c game.c map.c utils.c vector.c -I/usr/include/SDL2 -Ithirdparty -lSDL2 -lSDL2main -lSDL2_ttf -lm -o main -g

main_hd: $(DEPENDECIES)
	$(CC) $(CFLAGS) -DHIGH_RESOLUTION main.c game.c map.c utils.c vector.c -I/usr/include/SDL2 -Ithirdparty -lSDL2 -lSDL2main -lSDL2_ttf -lm -o main_hd -g

clean:
	rm -rf main
	rm -rf main_hd
	rm -rf *.o

