#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "utils.h"
#include "global.h"
#include "thread_funcs.h"

void* emulated_cpu_func(void*)
{
	int num_executions = 0;
	while (true)
	{
		/*
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
		*/
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
		sem_wait(&sems[3][READY_FOR_WRITE]);

		val >>= 8;
		val += ports[4] << 8;
		ports[3] = val >> (8 - (ports[2] & 7));

		sem_post(&sems[4][READY_FOR_WRITE]);
		sem_post(&sems[3][READY_FOR_READ]);
	}

	return NULL;
}
