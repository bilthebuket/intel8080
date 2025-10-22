#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "instructions.h"
#include "global.h"
#include "utils.h"

void mov(void)
{
	unsigned char src;
	unsigned char dest = (mem[IP] & THREE_TO_FIVE_BITS) >> 3;
	int num_cycles = 1;

	if (mem[IP] & (1 << 6))
	{
		src = mem[IP] & ZERO_TO_TWO_BITS;

		if (src == HL_MEM)
		{
			src = mem[get_rp(HL)];
			num_cycles++;
		}
		else
		{
			src = registers[src];
		}
		IP++;
	}
	else
	{
		src = mem[IP + 1];
		num_cycles++;
		IP += 2;
	}

	if (dest == HL_MEM)
	{
		mem[get_rp(HL)] = src;
		num_cycles++;
	}
	else
	{
		registers[dest] = src;
	}

	cycle_sleep(num_cycles);
}

void lxi(void)
{
	unsigned short val = mem[IP + 1] + (mem[IP + 2] << 8);
	set_rp((mem[IP] & 48) >> 4, val);
	IP += 3;
	cycle_sleep(3);
}

void lda(void)
{
	registers[A] = mem[(mem[IP + 2] << 8) + mem[IP + 1]];
	IP += 3;
	cycle_sleep(4);
}

void sta(void)
{
	mem[(mem[IP + 2] << 8) + mem[IP + 1]] = registers[A];
	IP += 3;
	cycle_sleep(4);
}

void lhld(void)
{
	registers[L] = mem[(mem[IP + 2] << 8) + (mem[IP + 1])];
	registers[H] = mem[(mem[IP + 2] << 8) + (mem[IP + 1]) + 1];
	IP += 3;
	cycle_sleep(5);
}

void shld(void)
{
	mem[(mem[IP + 2] << 8) + (mem[IP + 1])] = registers[L];
	mem[(mem[IP + 2] << 8) + (mem[IP + 1]) + 1] = registers[H];
	IP += 3;
	cycle_sleep(5);
}

void ldax(void)
{
	registers[A] = mem[get_rp((mem[IP] & 48) >> 4)];
	IP++;
	cycle_sleep(2);
}

void stax(void)
{
	mem[get_rp((mem[IP] & 48) >> 4)] = registers[A];
	IP++;
	cycle_sleep(2);
}

void xchg(void)
{
	unsigned char tmp = registers[H];
	registers[H] = registers[D];
	registers[D] = tmp;

	tmp = registers[L];
	registers[L] = registers[E];
	registers[E] = tmp;
	IP++;
	cycle_sleep(1);
}

void add(void)
{
	unsigned char store = registers[A];
	int num_cycles = 1;

	if ((mem[IP] & ZERO_TO_TWO_BITS) == 6)
	{
		registers[A] += mem[(registers[H] << 8) + registers[L]];
		num_cycles++;
	}
	else
	{
		registers[A] += registers[mem[IP] & ZERO_TO_TWO_BITS];
	}

	update_flags(store, registers[A], true);
	IP++;
	cycle_sleep(num_cycles);
}

void adi(void)
{
	unsigned char store = registers[A];
	registers[A] += mem[IP + 1];
	update_flags(store, registers[A], true);
	IP += 2;
	cycle_sleep(2);
}

void adc(void)
{
	unsigned char store = registers[A];
	int num_cycles = 1;

	if ((mem[IP] & ZERO_TO_TWO_BITS) == 6)
	{
		registers[A] += mem[(registers[H] << 8) + registers[L]] + !(!(flags & FLAG_C));
		num_cycles++;
	}
	else
	{
		registers[A] += registers[mem[IP] & ZERO_TO_TWO_BITS] + !(!(flags & FLAG_C));
	}

	update_flags(store, registers[A], true);
	IP++;
	cycle_sleep(num_cycles);
}

void aci(void)
{
	unsigned char store = registers[A];
	registers[A] += (mem[IP + 1]) + !(!(flags & FLAG_C));
	update_flags(store, registers[A], true);
	IP += 2;
	cycle_sleep(2);
}

void sub(void)
{
	unsigned char store = registers[A];
	int num_cycles = 1;

	if ((mem[IP] & ZERO_TO_TWO_BITS) == 6)
	{
		registers[A] -= mem[(registers[H] << 8) + registers[L]];
		num_cycles++;
	}
	else
	{
		registers[A] -= registers[mem[IP] & ZERO_TO_TWO_BITS];
	}

	update_flags(store, registers[A], false);
	IP++;
	cycle_sleep(num_cycles);
}

void sui(void)
{
	unsigned char store = registers[A];
	registers[A] -= mem[IP + 1];
	update_flags(store, registers[A], false);
	IP += 2;
	cycle_sleep(2);
}

void sbb(void)
{
	unsigned char store = registers[A];
	int num_cycles = 1;

	if ((mem[IP] & ZERO_TO_TWO_BITS) == 6)
	{
		registers[A] -= mem[(registers[H] << 8) + registers[L]] + !(!(flags & FLAG_C));
		num_cycles++;
	}
	else
	{
		registers[A] -= registers[mem[IP] & ZERO_TO_TWO_BITS] + !(!(flags & FLAG_C));
	}

	update_flags(store, registers[A], false);
	IP++;
	cycle_sleep(num_cycles);
}

void sbi(void)
{
	unsigned char store = registers[A];
	registers[A] -= (mem[IP + 1]) + !(!(flags & FLAG_C));
	update_flags(store, registers[A], false);
	IP += 2;
	cycle_sleep(2);
}

void inr(void)
{
	unsigned char store = flags & FLAG_C;
	int num_cycles = 1;
	
	if (((mem[IP] & THREE_TO_FIVE_BITS) >> 3) == 6)
	{
		mem[(registers[H] << 8) + registers[L]]++;
		update_flags(mem[(registers[H] << 8) + registers[L]] - 1, mem[(registers[H] << 8) + registers[L]], true);
		num_cycles += 2;
	}
	else
	{
		registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3]++;
		update_flags(registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3] - 1, registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3], true);
	}

	flags |= store;
	IP++;
	cycle_sleep(num_cycles);
}

void dcr(void)
{
	unsigned char store = flags & FLAG_C;
	int num_cycles = 1;

	if (((mem[IP] & THREE_TO_FIVE_BITS) >> 3) == 6)
	{
		mem[(registers[H] << 8) + registers[L]]--;
		update_flags(mem[(registers[H] << 8) + registers[L]] + 1, mem[(registers[H] << 8) + registers[L]], false);
		num_cycles += 2;
	}
	else
	{
		registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3]--;
		update_flags(registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3] + 1, registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3], false);
	}

	flags |= store;
	IP++;
	cycle_sleep(num_cycles);
}

void inx(void)
{
	unsigned short val = get_rp((mem[IP] & 48) >> 4);
	val++;
	set_rp((mem[IP] & 48) >> 4, val);
	IP++;
	cycle_sleep(1);
}

void dcx(void)
{
	unsigned short val = get_rp((mem[IP] & 48) >> 4);
	val--;
	set_rp((mem[IP] & 48) >> 4, val);
	IP++;
	cycle_sleep(1);
}

void dad(void)
{
	unsigned short start_value = get_rp(HL);
	unsigned short end_value = start_value + get_rp((mem[IP] & 48) >> 4);
	set_rp(HL, end_value);
	if (start_value > end_value)
	{
		flags |= FLAG_C;
	}
	IP++;
	cycle_sleep(3);
}

void daa(void)
{
	unsigned char store = registers[A];

	if (((registers[A] & 15) > 9) || (flags & FLAG_A))
	{
		registers[A] += 6;
	}
	if ((((registers[A] >> 4) & 15) > 9) || (flags & FLAG_C))
	{
		registers[A] += 96;
	}

	update_flags(store, registers[A], true);
	IP++;
	cycle_sleep(1);
}

void ana(void)
{
	unsigned char store = registers[A];
	int num_cycles = 1;

	if (mem[IP] == 166)
	{
		registers[A] &= mem[get_rp(HL)];
		num_cycles++;
	}
	else
	{
		registers[A] &= registers[mem[IP] & 7];
	}

	update_flags(store, registers[A], true);
	flags &= (255 - FLAG_C);
	IP++;
	cycle_sleep(num_cycles);
}

void ani(void)
{
	unsigned char store = registers[A];
	registers[A] &= mem[IP + 1];
	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP += 2;
	cycle_sleep(2);
}

void xra(void)
{
	unsigned char store = registers[A];
	int num_cycles = 1;

	if (mem[IP] == 174)
	{
		registers[A] ^= mem[get_rp(HL)];
		num_cycles++;
	}
	else
	{
		registers[A] ^= registers[mem[IP] & 7];
	}

	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP++;
	cycle_sleep(num_cycles);
}

void xri(void)
{
	unsigned char store = registers[A];
	registers[A] ^= mem[IP + 1];
	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP += 2;
	cycle_sleep(2);
}

void ora(void)
{
	unsigned char store = registers[A];
	int num_cycles = 1;

	if (mem[IP] == 182)
	{
		registers[A] |= mem[get_rp(HL)];
		num_cycles++;
	}
	else
	{
		registers[A] |= registers[mem[IP] & 7];
	}

	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP++;
	cycle_sleep(num_cycles);
}

void ori(void)
{
	unsigned char store = registers[A];
	registers[A] |= mem[IP + 1];
	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP += 2;
	cycle_sleep(2);
}

void cmp(void)
{
	unsigned char subtractor;
	int num_cycles = 1;

	if (mem[IP] == 190)
	{
		subtractor = mem[get_rp(HL)];
		num_cycles++;
	}
	else
	{
		subtractor = registers[mem[IP] & 7];
	}
	update_flags(registers[A], registers[A] - subtractor, false);
	IP++;
	cycle_sleep(num_cycles);
}

void cpi(void)
{
	update_flags(registers[A], registers[A] - (mem[IP + 1]), false);
	IP += 2;
	cycle_sleep(2);
}

void rlc(void)
{
	unsigned char num = registers[A] >> 7;
	registers[A] <<= 1;
	if (num)
	{
		registers[A] |= num;
		flags |= FLAG_C;
	}
	else
	{
		registers[A] &= 254;
		flags &= 255 - FLAG_C;
	}
	IP++;
	cycle_sleep(1);
}

void rrc(void)
{
	unsigned char num = registers[A] << 7;
	registers[A] >>= 1;
	registers[A] |= num;
	if (num)
	{
		flags |= FLAG_C;
	}
	else
	{
		flags &= 255 - FLAG_C;
	}
	IP++;
	cycle_sleep(1);
}

void ral(void)
{
	unsigned char store = 0;
	if (flags & FLAG_C)
	{
		store = 1;
	}

	if (registers[A] & 128)
	{
		flags |= FLAG_C;
	}
	else
	{
		flags &= 255 - FLAG_C;
	}

	registers[A] <<= 1;
	registers[A] = store | (registers[A] & 254);
	IP++;
	cycle_sleep(1);
}

void rar(void)
{
	unsigned char store = 0;
	if (flags & FLAG_C)
	{
		store = 128;
	}

	if (registers[A] & 1)
	{
		flags |= FLAG_C;
	}
	else
	{
		flags &= 255 - FLAG_C;
	}

	registers[A] >>= 1;
	registers[A] = store | (registers[A] & 127);
	IP++;
	cycle_sleep(1);
}

void cma(void)
{
	registers[A] = ~registers[A];
	IP++;
	cycle_sleep(1);
}

void cmc(void)
{
	flags ^= FLAG_C;
	IP++;
	cycle_sleep(1);
}

void stc(void)
{
	flags |= FLAG_C;
	IP++;
	cycle_sleep(1);
}

void jmp(void)
{
	IP = (mem[IP + 2] << 8) + (mem[IP + 1]);
	cycle_sleep(3);
}

void jcon(void)
{
	if ((*condition_checks[(mem[IP] & 56) >> 3])())
	{
		IP = (mem[IP + 2] << 8) + (mem[IP + 1]);
	}
	else
	{
		IP += 3;
	}
	cycle_sleep(3);
}

void call(void)
{
	mem[SP - 1] = (IP + 3) >> 8;
	mem[SP - 2] = (IP + 3) & 255;
	SP -= 2;
	IP = (mem[IP + 2] << 8) + (mem[IP + 1]);
	cycle_sleep(5);
}

void ccall(void)
{
	if ((*condition_checks[(mem[IP] & 56) >> 3])())
	{
		mem[SP - 1] = (IP + 3) >> 8;
		mem[SP - 2] = (IP + 3) & 255;
		SP -= 2;
		IP = (mem[IP + 2] << 8) + (mem[IP + 1]);
		cycle_sleep(5);
	}
	else
	{
		IP += 3;
		cycle_sleep(3);
	}
}

void ret(void)
{
	IP = 0;
	IP += mem[SP] + (mem[SP + 1] << 8);
	SP += 2;
	cycle_sleep(3);
}

void cret(void)
{
	if ((*condition_checks[(mem[IP] & 56) >> 3])())
	{
		IP = 0;
		IP += mem[SP] + (mem[SP + 1] << 8);
		SP += 2;
		cycle_sleep(3);
	}
	else
	{
		IP++;
		cycle_sleep(1);
	}
}

void rst(void)
{
	mem[SP - 1] = (IP + 1) >> 8;
	mem[SP - 2] = (IP + 1) & 255;
	SP -= 2;
	IP = rst_addrs[(mem[IP] & 56) >> 3];
	cycle_sleep(3);
}

void pchl(void)
{
	IP = registers[H] << 8;
	IP += registers[L];
	cycle_sleep(1);
}

void push(void)
{
	mem[SP - 1] = registers[register_pairs[(mem[IP] & 48) >> 4] >> 3];
	mem[SP - 2] = registers[register_pairs[(mem[IP] & 48) >> 4] & 7];
	SP -= 2;
	IP++;
	cycle_sleep(3);
}

void pushp(void)
{
	mem[SP - 1] = registers[A];
	int val = 2;
	val += (flags & FLAG_C) >> 3;
	val += (flags & FLAG_P);
	val += (flags & FLAG_A);
	val += (flags & FLAG_Z) << 6;
	val += (flags & FLAG_S) << 6;
	mem[SP - 2] = val;
	SP -= 2;
	IP++;
	cycle_sleep(3);
}

void pop(void)
{
	registers[register_pairs[(mem[IP] & 48) >> 4] >> 3] = mem[SP + 1];
	registers[register_pairs[(mem[IP] & 48) >> 4] & 7] = mem[SP];
	SP += 2;
	IP++;
	cycle_sleep(3);
}

void popp(void)
{
	flags = 0;
	flags += (mem[SP] & 1) << FLAG_C_BIT;
	flags += (mem[SP] & 4);
	flags += (mem[SP] & 16);
	flags += (mem[SP] & 64) >> 6;
	flags += (mem[SP] & 128) >> 6;
	registers[A] = mem[SP + 1];
	SP += 2;
	IP++;
	cycle_sleep(3);
}

void xthl(void)
{
	unsigned char temp;
	temp = registers[L];
	registers[L] = mem[SP];
	mem[SP] = temp;
	temp = registers[H];
	registers[H] = mem[SP + 1];
	mem[SP + 1] = temp;
	IP++;
	cycle_sleep(5);
}

void sphl(void)
{
	SP = (registers[H] << 8) + registers[L];
	IP++;
	cycle_sleep(1);
}

void in(void)
{
	sem_wait(&sems[mem[IP + 1]]);
	registers[A] = ports[mem[IP + 1]];
	sem_post(&sems[mem[IP + 1]]);
	IP += 2;
	cycle_sleep(3);
}

void out(void)
{
	static int val = 0;
	sem_wait(&sems[mem[IP + 1]]);
	ports[mem[IP + 1]] = registers[A];
	has_been_updated |= 1 << 4;

	// shift register
	if (mem[IP + 1] == 4)
	{
		val >>= 8;
		val += ports[4] << 8;
		ports[3] = val >> (8 - (ports[2] & 7));
	}

	sem_post(&sems[mem[IP + 1]]);
	IP += 2;
	cycle_sleep(3);
}

void ei(void)
{
	can_interrupt = true;
	IP++;
	cycle_sleep(1);
}

void di(void)
{
	can_interrupt = false;
	actually_can_interrupt = false;
	IP++;
	cycle_sleep(1);
}

void hlt(void)
{
	printf("Halted\n");
	IP++;
	cycle_sleep(1);
}

void nop(void)
{
	IP++;
	cycle_sleep(1);
}

void invalid(void)
{
	printf("Invalid instruction!\n");
	IP++;
}
