#include <stdio.h>
#include "operations.h"

#define MEMORY_SIZE 65536

#define NUM_REGISTER_ENCODINGS 8;
#define NUM_REGISTER_PAIR_ENCODINGS 4;

// indexs of different registers in the registers array
#define A 7
#define B 0
#define C 1
#define D 2
#define E 3
#define H 4
#define L 5
#define H-L_MEM 6

#define B-C 00
#define D-E 01
#define H-L 10
#define S-P 11

// zero, sign, parity, carry, and auxiliary carry flags
#define FLAG_Z (0) // if the result of an operation is zero
#define FLAG_S (1 << 1) // if the result of an operation leads to the sign bit (most siginificant bit) being 1
#define FLAG_P (1 << 2) // if the reuslt of an operation is even
#define FLAG_C (1 << 3) // if there is a wrap around
#define FLAG_A (1 << 4) // if there was a carry out of bit 3 into bit 4
			//
#define NUM_INSTRUCTIONS 255;

// a single assembly instruction, where the unsigned char is the actual instruction and unsigned short
// contains the subsquent data/memory address
typedef void (*instruction)(unsigned char, unsigned short);

// registers
unsigned char registers[REGISTER_MAXIMUM_ENCODING_VALUE];
unsigned char register_pairs[NUM_REGISTER_PAIR_ENCODINGS];
unsigned short SP;
unsigned short IP;

unsigned char flags;

unsigned char mem[MEMORY_SIZE];

instruction instructions[NUM_INSTRUCTIONS];

/*
 * initialize the array of instructions and register pair array
*/
void initialize(void);

void mov(unsigned char operation, unsigned short data);
void lxi(unsigned char operation, unsigned short data);
void lda(unsigned char operation, unsigned short data);

int main(void)
{
	flags = 0;
}

void initialize(void)
{
	register_pairs[B-C] = (B << 3) + C;
	register_pairs[D-E] = (D << 3) + E;
	register_pairs[H-L] = (H << 3) + L;
	register_pairs[S-P] = (S << 3) + P;

	for (int i = 64; i < 128; i++)
	{
		instructions[i] = &mov;
	}
	for (int i = 6; i < 63; i++)
	{
		instructions[i] = &mov;
	}

	instructions[1] = &lxi;
	instructions[17] = &lxi;
	instructions[33] = &lxi;
	instructions[49] = &lxi;
}

void mov(unsigned char operation, unsigned short data)
{
	if (operation == 118) // HLT
	{
		return;
	}

	unsigned char src;
	unsigned char dest = (operation & 56) >> 3;;

	if (operation & (1 << 6))
	{
		src = operation & 7;

		if (src == H-L_MEM)
		{
			src = mem[registers[H] << 8 + registers[L]];
		}
		else
		{
			src = registers[src];
		}
	}
	else
	{
		src = data >> 8;
	}

	if (dest == H-L_MEM)
	{
		mem[registers[H] << 8 + registers[L]] = src;
	}
	else
	{
		registers[dest] = src;
	}
}

void lxi(unsigned char operation, unsigned short data)
{
	registers[register_pairs[operation & 48] & 7] = data >> 8;
	registers[register_pairs[operation & 48] & 56] = data & 255;
}

void lda(unsigned char operation, unsigned short data)
{
	
}
