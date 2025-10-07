#include <stdio.h>
#include "instructions.h"
#include "global.h"
#include "utils.h"

void run_tests(void);

int main(int argc, char* argv[])
{
	initialize_arrays();
	run_tests();
}

void run_tests(void)
{
	update_flags(127, 128, true);
	if (flags != FLAG_S + FLAG_A)
	{
		printf("Flag test 1 failed. Expected %d, found %d.\n", FLAG_S + FLAG_P + FLAG_C + FLAG_A, flags);
	}
	else
	{
		printf("Flag test 1 passed.\n");
	}

	update_flags(1, 0, false);
	if (flags != FLAG_Z + FLAG_P)
	{
		printf("Flag test 2 failed. Expected %d, found %d.\n", FLAG_Z + FLAG_P, flags);
	}
	else
	{
		printf("Flag test 2 passed.\n");
	}

	update_flags(255, 5, true);
	if (flags != FLAG_C + FLAG_A + FLAG_P)
	{
		printf("Flag test 3 failed. Expected %d, found %d.\n", FLAG_C + FLAG_A, flags);
	}
	else
	{
		printf("Flag test 3 passed.\n");
	}

	update_flags(1, 0, false);
	if (flags != FLAG_Z + FLAG_P)
	{
		printf("Flag test 4 failed.\n");
	}
	else
	{
		printf("Flag test 4 passed.\n");
	}

	update_flags(1, 255, false);
	if (flags != FLAG_S + FLAG_C + FLAG_A + FLAG_P)
	{
		printf("Flag test 5 failed.\n");
	}
	else
	{
		printf("Flag test 5 passed.\n");
	}

	update_flags(0, 3, true);
	if (flags != FLAG_P)
	{
		printf("Flag test 6 failed.\n");
	}
	else
	{
		printf("Flag test 6 passed.\n");
	}

	update_flags(255, 0, true);
	if (flags != FLAG_Z + FLAG_C + FLAG_A + FLAG_P)
	{
		printf("Flag test 7 failed.\n");
	}
	else
	{
		printf("Flag test 7 passed.\n");
	}

	update_flags(0, 255, false);
	if (flags != FLAG_S + FLAG_C + FLAG_A + FLAG_P)
	{
		printf("Flag test 8 failed.\n");
	}
	else
	{
		printf("Flag test 8 passed.\n");
	}

	update_flags(15, 16, true);
	if (flags != FLAG_A)
	{
		printf("Flag test 9 failed.\n");
	}
	else
	{
		printf("Flag test 9 passed.\n");
	}

	update_flags(16, 15, false);
	if (flags != FLAG_A + FLAG_P)
	{
		printf("Flag test 10 failed.\n");
	}
	else
	{
		printf("Flag test 10 passed.\n");
	}

	update_flags(127, 127, true);
	if (flags != 0)
	{
		printf("Flag test 11 failed.\n");
	}
	else
	{
		printf("Flag test 11 passed.\n");
	}

	IP = 0;
	
	registers[A] = 0;
	registers[B] = 1;
	mem[IP] = (1 << 6) + (A << 3) + B;
	(*instructions[mem[IP]])();
	if (registers[A] != 1)
	{
		printf("Test 1 (mov r1, r2) failed.\n");
	}
	else
	{
		printf("Test 1 (mov r1, r2) passed.\n");
	}
	IP = 0;

	registers[A] = 5;
	registers[H] = 1;
	registers[L] = 12;
	mem[(1 << 8) + 12] = 6;
	mem[IP] = (1 << 6) + (3 << 1) + (A << 3);
	(*instructions[mem[IP]])();
	if (registers[A] != 6)
	{
		printf("Test 2 (mov r, M) failed.\n");
	}
	else
	{
		printf("Test 2 (mov r, M) passed.\n");
	}
	IP = 0;

	mem[(1 << 8) + 12] = 2;
	mem[IP] = (7 << 4) + E;
	registers[E] = 11;
	(*instructions[mem[IP]])();
	if (mem[(1 << 8) + 12] != 11)
	{
		printf("Test 3 (mov M, r) failed.\n");
	}
	else
	{
		printf("Test 3 (mov M, r) passed.\n");
	}
	IP = 0;

	registers[C] = 1;
	mem[IP] = (3 << 1) + (C << 3);
	mem[IP + 1] = 16;
	(*instructions[mem[IP]])();
	if (registers[C] != 16)
	{
		printf("Test 4 (mov r, data) failed.\n");
	}
	else
	{
		printf("Test 4 (mov r, data) passed.\n");
	}
	IP = 0;

	registers[H] = 1;
	registers[L] = 12;
	mem[(1 << 8) + 12] = 0;
	mem[IP + 1] = 1;
	mem[IP] = (3 << 1) + (3 << 4);
	(*instructions[mem[IP]])();
	if (mem[(1 << 8) + 12] != 1)
	{
		printf("Test 5 (mov M, data) failed.\n");
	}
	else
	{
		printf("Test 5 (mov M, data) passed.\n");
	}
	IP = 0;

	mem[IP + 1] = 19;
	mem[IP + 2] = 20;
	mem[IP] = (DE << 4) + 1;
	registers[D] = 0;
	registers[E] = 0;
	(*instructions[mem[IP]])();
	if (registers[D] != 20 || registers[E] != 19)
	{
		printf("Test 6 (lxi rp, data) failed.\n");
	}
	else
	{
		printf("Test 6 (lxi rp, data) passed.\n");
	}
	IP = 0;

	mem[IP] = (7 << 3) + 2;
	mem[IP + 1] = 132;
	mem[IP + 2] = 43;
	mem[(43 << 8) + 132] = 8;
	registers[A] = 0;
	(*instructions[mem[IP]])();
	if (registers[A] != 8)
	{
		printf("Test 7 (lda addr) failed. Expected 8 found %d.\n", registers[A]);
	}
	else
	{
		printf("Test 7 (lda addr) passed.\n");
	}
	IP = 0;

	mem[IP] = 50;
	mem[IP + 1] = 127;
	mem[IP + 2] = 202;
	mem[(202 << 8) + 127] = 67;
	registers[A] = 68;
	(*instructions[mem[IP]])();
	if (mem[(202 << 8) + 127] != 68)
	{
		printf("Test 8 (sta addr) failed. Expected 68 found %d.\n", mem[(202 << 8) + 127]);
	}
	else
	{
		printf("Test 8 (sta addr) passed.\n");
	}
	IP = 0;

	mem[IP] = 42;
	mem[IP + 1] = 50;
	mem[IP + 2] = 89;
	registers[H] = 12;
	registers[L] = 13;
	mem[(89 << 8) + 50] = 14;
	mem[(89 << 8) + 51] = 15;
	(*instructions[mem[IP]])();
	if (registers[H] != 15 || registers[L] != 14)
	{
		printf("Test 9 (lhld addr) failed. Expected 15 in register H and 14 in register L, found %d and %d respectively.\n", registers[H], registers[L]);
	}
	else
	{
		printf("Test 9 (lhld addr) passed.\n");
	}
	IP = 0;

	mem[IP] = 34;
	mem[IP + 1] = 14;
	mem[IP + 2] = 12;
	mem[(12 << 8) + 14] = 60;
	mem[(12 << 8) + 15] = 62;
	registers[H] = 1;
	registers[L] = 0;
	(*instructions[mem[IP]])();
	if (mem[(12 << 8) + 14] != 0 || mem[(12 << 8) + 15] != 1)
	{
		printf("Test 10 (shld addr) failed. Expected 62 in register H and 60 in register L, found %d and %d respectively.\n", registers[H], registers[L]);
	}
	else
	{
		printf("Test 10 (shld addr) passed.\n");
	}
	IP = 0;

	mem[IP] = 10;
	registers[B] = 49;
	registers[C] = 12;
	mem[(registers[B] << 8) + registers[C]] = 9;
	registers[A] = 1;
	(*instructions[mem[IP]])();
	if (registers[A] != 9)
	{
		printf("Test 11 (ldax rp) failed. Expected 9 in register A, found %d.\n", registers[A]);
	}
	else
	{
		printf("Test 11 (ldax rp) passed.\n");
	}
	IP = 0;

	mem[IP] = 18;
	registers[D] = 50;
	registers[E] = 12;
	mem[(registers[D] << 8) + registers[E]] = 1;
	registers[A] = 0;
	(*instructions[mem[IP]])();
	if (mem[(registers[D] << 8) + registers[E]] != 0)
	{
		printf("Test 12 (stax rp) failed. Expected 0 in memory location %d, found %d.\n", (registers[D] << 8) + registers[E], mem[(registers[D] << 8) + registers[E]]);
	}
	else
	{
		printf("Test 12 (stax rp) passed.\n");
	}
	IP = 0;

	mem[IP] = 235;
	registers[H] = 5;
	registers[L] = 6;
	registers[D] = 7;
	registers[E] = 8;
	(*instructions[mem[IP]])();
	if (registers[H] != 7 || registers[D] != 5 || registers[L] != 8 || registers[E] != 6)
	{
		printf("Test 13 (xchg) failed.\n");
	}
	else
	{
		printf("Test 13 (xchg passed.\n");
	}
	IP = 0;

	mem[IP] = 128 + C;
	registers[C] = 5;
	registers[A] = 1;
	(*instructions[mem[IP]])();
	if (registers[A] != 6)
	{
		printf("Test 14 (add r) failed. Expected 6, found %d.\n", registers[A]);
	}
	else
	{
		printf("Test 14 (add r) passed.\n");
	}
	IP = 0;

	mem[IP] = 134;
	mem[(registers[H] << 8) + registers[L]] = 12;
	registers[A] = 5;
	(*instructions[mem[IP]])();
	if (registers[A] != 17)
	{
		printf("Test 15 (add m) failed. Expected 17, found %d.\n", registers[A]);
	}
	else
	{
		printf("Test 15 (add m) passed.\n");
	}
	IP = 0;
}
