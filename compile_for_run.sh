gcc -Iinclude -g src/emulate.c src/global.c src/utils.c src/instructions.c -o emulate $(sdl2-config --cflags --libs)
