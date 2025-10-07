#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.h"
#include "global.h"

void initialize(void);

void* shift_register_func(void*);
void* emulated_cpu_func(void*);

int main(int argc, char* argv)
{
	initialize();

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("SDL init error\n");
		return 1;
	}

	SDL_Window* win = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 224, 256, SDL_WINDOW_SHOWN);
	if (!win)
	{
		printf("SDL create window error\n");
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
	{
		printf("SDL create renderer error\n");
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}

	int running = 1;
	SDL_Event event;

	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			// i dont think this semaphore implementation is correct
			// the mutex is fine but i think it might lead to lots of lag when
			// playing the game, idk though ts dont even work at all rn
			sem_wait(&sems[1][READY_FOR_WRITE]);

			if (event.type == SDL_QUIT)
			{
				running = 0;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
					case 'a':
						ports[1] |= 1 << 5;
						break;

					case 'd':
						ports[1] |= 1 << 6;
						break;

					case ' ':
						ports[1] |= 1 << 4;
						break;

					case 'c':
						ports[1] |= 1;
						break;
				}
			}
			else if (event.type == SDL_KEYUP)
			{
				switch (event.key.keysym.sym)
				{
					case 'a':
						ports[1] &= (255 ^ (1 << 5));
						break;

					case 'd':
						ports[1] &= (255 ^ (1 << 6));
						break;

					case ' ':
						ports[1] &= (255 ^ (1 << 4));
						break;

					case 'c':
						ports[1] &= 254;
						break;
				}
			}
			sem_post(&sems[1][READY_FOR_READ]);
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		
		for (int i = 0x2400; i <= 0x3FE0; i += 0x20)
		{
			for (int j = i; j <= i + 0x1F; j++)
			{
				for (int k = 0; k < 8; k++)
				{
					if (mem[j] & (1 << k))
					{
						SDL_RenderDrawPoint(renderer, (j - i) * 8 + k, (i - 0x2400) % 0x20);
					}
				}
			}
		}

		SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}

void initialize(void)
{
	initialize_arrays();

	int fd = open("invaders/invaders.h", O_RDONLY);
	read(fd, &mem[0], 2048);
	close(fd);

	fd = open("invaders/invaders.g", O_RDONLY);
	read(fd, &mem[2048], 2048);
	close(fd);
	
	fd = open("invaders/invaders.f", O_RDONLY);
	read(fd, &mem[4096], 2048);
	close(fd);

	fd = open("invaders/invaders.e", O_RDONLY);
	read(fd, &mem[6144], 2048);
	close(fd);
	int value;

	if (pthread_create(&shift_register_thread, NULL, &shift_register_func, &value) != 0)
	{
		printf("Could not create shift register thread\n");
		exit(1);
	}

	if (pthread_create(&emulated_cpu_thread, NULL, &emulated_cpu_func, &value) != 0)
	{
		printf("Could not create cpu thread\n");
		exit(1);
	}
}

void* emulated_cpu_func(void*)
{
	int num_executions = 0;
	while (true)
	{
		printf("DE: %d\n", get_rp(DE));
		printf("HL: %d\n", get_rp(HL));
		printf("Exec num: %d\n", num_executions);
		printf("Instruction: %d\n", mem[IP]);
		printf("Program Counter: %d\n\n", IP);
		if (IP >= 9216 && IP <= 16383)
		{
			printf("IP has reached somewhere it shouldn't be\n");
			exit(1);
		}
		fflush(stdout);
		(*instructions[mem[IP]])();
		num_executions++;

	}

	return NULL;
}

void* shift_register_func(void*)
{
	unsigned short val = 0;
	while (true)
	{
		sem_wait(&sems[4][READY_FOR_READ]);

		if (has_been_updated & (1 << 4))
		{
			val >>= 8;
			val += ports[4] << 8;
			has_been_updated ^= 1 << 4;
		}
		
		sem_post(&sems[4][READY_FOR_WRITE]);

		sem_wait(&sems[2][READY_FOR_READ]);
		sem_wait(&sems[3][READY_FOR_WRITE]);

		ports[3] = val >> (8 - (ports[2] & 7));

		sem_post(&sems[2][READY_FOR_WRITE]);
		sem_post(&sems[3][READY_FOR_READ]);
	}

	return NULL;
}
