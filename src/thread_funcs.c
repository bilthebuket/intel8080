#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "utils.h"
#include "global.h"
#include "thread_funcs.h"
#include "instructions.h"

void print_test_info(void)
{
	if (registers[C] == 2)
	{
		printf("%c", registers[E]);
	}
	else if (registers[C] == 9)
	{
		unsigned short addr = (registers[D] << 8) | registers[E];
		for (; mem[addr] != '$'; addr++)
		{
			printf("%c", mem[addr]);
		}
	}
	else if (registers[C] == 0)
	{
		exit(0);
	}
}

void* emulated_cpu_func(void*)
{
	int num_executions = 0;
	while (true)
	{
		/*
		printf("DE: %X\n", get_rp(DE));
		printf("HL: %X\n", get_rp(HL));
		printf("Exec num: %d\n", num_executions);
		printf("Instruction: %X\n", mem[IP]);
		printf("Program Counter: %X\n\n", IP);
		if (IP >= 9216 && IP <= 16383)
		{
			printf("IP has reached somewhere it shouldn't be\n");
			exit(1);
		}
		*/
		if (IP == 5)
		{
			print_test_info();
			ret();
		}
		else if (IP == 0x06A0)
		{
			int do_something = 5;
		}
		else if (num_executions == 370)
		{
			(*instructions[mem[IP]])();
		}
		else
		{
			(*instructions[mem[IP]])();
		}
		fflush(stdout);
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
