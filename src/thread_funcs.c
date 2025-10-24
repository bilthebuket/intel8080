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
	int interrupt = 1;
	while (true)
	{
		//printf("%lf\n", num_cycles_executed);
		//print_debug_info();

		// checking for hardware interrupt (i'm using port 7 to send hardware interrupts from the screen to the cpu because space invaders didn't use it)
		if (actually_can_interrupt)
		{
			if (num_cycles_executed >= CYCLES_PER_INTERRUPT)
			{
				//printf("Interrupt %d!\n", interrupt);
				if (mem[IP] == 118)
				{
					IP++;
				}

				//printf("Interrupt from: %X to: ", IP);

				if (interrupt == 1)
				{

					mem[SP - 1] = IP >> 8;
					mem[SP - 2] = IP & 255;
					SP -= 2;
					IP = rst_addrs[1];
					cycle_sleep(11);
				}
				else
				{
					mem[SP - 1] = IP >> 8;
					mem[SP - 2] = IP & 255;
					SP -= 2;
					IP = rst_addrs[2];
					cycle_sleep(11);
				}

				can_interrupt = false;
				actually_can_interrupt = false;

				//printf("%X\n", IP);
			}
			/*
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
			*/
		}

		if (num_cycles_executed >= CYCLES_PER_INTERRUPT)
		{
			num_cycles_executed -= CYCLES_PER_INTERRUPT;
			if (interrupt == 1)
			{
				interrupt = 2;
			}
			else
			{
				interrupt = 1;
			}
		}

		if (mem[IP] == 118) // HLT
		{
			printf("Halt\n");
			cycle_sleep(7);
			continue;
		}

		/*
		*/
		if (IP > 0x1FFF)
		{
			printf("IP has reached somewhere it shouldn't be (%X)\n", IP);
			exit(1);
		}
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

		if (can_interrupt && !actually_can_interrupt)
		{
			actually_can_interrupt = true;
		}
	}

	return NULL;
}

/*
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
*/
