Intel 8080 emulator used to run space invaders

CPU runs at 2.048 MHz, game runs at 60 fps.
The program uses two threads, one for the display and one to emulate the cpu

--> controls <--

c = enter coin
1 = start game (1 player)
a = move left (player 1)
d = move right (player 1)
space = fire (player 1)

--> files <--

emulate.c: main() function, SDL2 window
global.c: global variables
instructions.c: functions that emulate each of the cpu instructions
thread_funcs.c: the cpu thread functions
utils.c: misc. helper functions
tests.c: for testing the shift register, cpu etc.
TST8080.COM: a ROM that tests all the different cpu instructions
