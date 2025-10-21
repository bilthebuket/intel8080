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
	while (true)
	{
		if (mem[IP] == 118) // HLT
		{
			continue;
		}
		
		// checking for hardware interrupt (i'm using port 7 to send hardware interrupts from the screen to the cpu because space invaders didn't use it)
		if (can_interrupt)
		{
			sem_wait(&sems[7]);
			if (ports[7] == 1)
			{
				mem[SP - 1] = (IP + 1) >> 8;
				mem[SP - 2] = (IP + 1) & 255;
				SP -= 2;
				IP = rst_addrs[1];
				ports[7] = 0;
				cycle_sleep(3);
			}
			else if (ports[7] == 2)
			{
				mem[SP - 1] = (IP + 1) >> 8;
				mem[SP - 2] = (IP + 1) & 255;
				SP -= 2;
				IP = rst_addrs[2];
				ports[7] = 0;
				cycle_sleep(3);
			}
			sem_post(&sems[7]);
		}

		/*
		printf("Zero Flag: %X\n", flags & FLAG_Z);
		printf("Sign Flag: %X\n", (flags & FLAG_S) >> 1);
		printf("Parity Flag: %X\n", (flags & FLAG_P) >> 2);
		printf("Carry Flag: %X\n", (flags & FLAG_C) >> 3);
		printf("Auxiliary Carry Flag: %X\n", (flags & FLAG_A) >> 4);
		printf("A: %X\n", registers[A]);
		printf("B: %X\n", registers[B]);
		printf("C: %X\n", registers[C]);
		printf("D: %X\n", registers[D]);
		printf("E: %X\n", registers[E]);
		printf("H: %X\n", registers[H]);
		printf("L: %X\n", registers[L]);
		printf("BC: %X\n", get_rp(BC));
		printf("DE: %X\n", get_rp(DE));
		printf("HL: %X\n", get_rp(HL));
		printf("SP: %X\n", SP);
		printf("{BC}: %X\n", mem[get_rp(BC)]);
		printf("{DE}: %X\n", mem[get_rp(DE)]);
		printf("{HL}: %X\n", mem[get_rp(HL)]);
		printf("{SP}: %X\n", mem[SP]);
		printf("Program Counter: %X\n", IP);
		printf("Instruction: %X: %s\n", mem[IP], names[mem[IP]]);
		printf("Exec num: %d\n\n", num_executions);
		if (IP >= 9216 && IP <= 16383)
		{
			printf("IP has reached somewhere it shouldn't be\n");
			exit(1);
		}
		*/
		// this is for running TST8080.COM
		/*
		if (IP == 5)
		{
			print_test_info();
			ret();
		}
		else if (instructions[mem[IP]] == &in || instructions[mem[IP]] == &out)
		{
			exit(0);
		}
		else if (IP == 0x06A0)
		{
			int do_something = 5;
		}
		else if (num_executions == 652)
		{
			(*instructions[mem[IP]])();
		}
		else
		{
			(*instructions[mem[IP]])();
		}
		*/
		(*instructions[mem[IP]])();
		fflush(stdout);
		num_executions++;
	}

	return NULL;
}

void* shift_register_func(void*)
{
	unsigned short val = 0;
	unsigned char port_4_val = 0;
	unsigned char port_2_val = 0;

	while (true)
	{
		sem_wait(&sems[4]);
		sem_wait(&sems[3]);

		if (ports[4] != port_4_val)
		{
			port_4_val = ports[4];
			val >>= 8;
			val += ports[4] << 8;
			ports[3] = val >> (8 - (ports[2] & 7));
		}
		if (ports[2] != port_2_val)
		{
			port_2_val = ports[2];
			ports[3] = val >> (8 - (ports[2] & 7));
		}

		sem_post(&sems[4]);
		sem_post(&sems[3]);
	}

	return NULL;
}
