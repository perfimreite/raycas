main: main.c
	gcc -Wall -Wextra -std=c11 main.c -I/usr/include/SDL2 -L/usr/lib -lSDL2 -lSDL2main -lm -o build/main && build/main

