#include <stdio.h>
#include <stdbool.h>

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
#define FLAG_Z (1) // if the result of an operation is zero
#define FLAG_S (1 << 1) // if the result of an operation leads to the sign bit (most siginificant bit) being 1
#define FLAG_P (1 << 2) // if the reuslt of an operation is even
#define FLAG_C (1 << 3) // if there is a wrap around
#define FLAG_A (1 << 4) // if there was a carry out of bit 3 into bit 4

#define FLAG_Z_BIT 0
#define FLAG_S_BIT 1
#define FLAG_P_BIT 2
#define FLAG_C_BIT 3
#define FLAG_A_BIT 4

#define NUM_INSTRUCTIONS 255
#define NUM_CONDITIONS 8

#define ZERO_TO_TWO_BITS 7
#define THREE_TO_FIVE_BITS 56

// a single assembly instruction, where the unsigned char is the actual instruction and unsigned short
// contains the subsquent data/memory address
typedef void (*instruction)(unsigned char, unsigned short);

// checks a condition (zero, not zero, carry, not carry, etc)
typedef bool (*condition_check)(void);

// registers
unsigned char registers[REGISTER_MAXIMUM_ENCODING_VALUE];

// each of the elements of the array are a 8 bit int that looks like this:
// GGHHHLLL
// where GG is a garbage value, HHH is the index in registers of the higher order register, 
// and LLL is the index in registers of the lower order register
unsigned char register_pairs[NUM_REGISTER_PAIR_ENCODINGS];

unsigned short SP;
unsigned short IP;

unsigned char flags = 0;

unsigned char mem[MEMORY_SIZE];

instruction instructions[NUM_INSTRUCTIONS];

condition_check condition_checks[NUM_CONDITIONS];

/*
 * initialize the array of instructions and register pair array
*/
void initialize(void);

void update_flags(unsigned char prev, unsigned char post, bool add);

unsigned short get_rp(unsigned char rp_index);
void set_rp(unsigned char rp_index, unsigned short val);

void mov(unsigned char operation, unsigned short data);
void lxi(unsigned char operation, unsigned short data);
void lda(unsigned char operation, unsigned short data);
void sta(unsigned char operation, unsigned short data);
void lhld(unsigned char operation, unsigned short data);
void shld(unsigned char operation, unsigned short data);
void ldax(unsigned char operation, unsigned short data);
void stax(unsigned char operation, unsigned short data);
void xchg(unsigned char operation, unsigned short data);
void add(unsigned char operation, unsigned short data);
void adi(unsigned char operation, unsigned short data);
void adc(unsigned char operation, unsigned short data);
void aci(unsigned char operation, unsigned short data);
void sub(unsigned char operation, unsigned short data);
void sui(unsigned char operation, unsigned short data);
void sbb(unsigned char operation, unsigned short data);
void sbi(unsigned char operation, unsigned short data);
void inr(unsigned char operation, unsigned short data);
void dcr(unsigned char operation, unsigned short data);
void inx(unsigned char operation, unsigned short data);
void dcx(unsigned char operation, unsigned short data);
void dad(unsigned char operation, unsigned short data);
void daa(unsigned char operation, unsigned short data);
void ana(unsigned char operation, unsigned short data);
void ani(unsigned char operation, unsigned short data);
void xra(unsigned char operation, unsigned short data);
void xri(unsigned char operation, unsigned short data);
void ora(unsigned char operation, unsigned short data);
void ori(unsigned char operation, unsigned short data);
void cmp(unsigned char operation, unsigned short data);
void cpi(unsigned char operation, unsigned short data);
void rlc(unsigned char operation, unsigned short data);
void rrc(unsigned char operation, unsigned short data);
void ral(unsigned char operation, unsigned short data);
void rar(unsigned char operation, unsigned short data);
void cma(unsigned char operation, unsigned short data);
void cmc(unsigned char operation, unsigned short data);
void stc(unsigned char operation, unsigned short data);
void jmp(unsigned char operation, unsigned short data);
void jcon(unsigned char operation, unsigned short data);
void call(unsigned char operation, unsigned short data);
void ccall(unsigned char operation, unsigned short data);
void ret(unsigned char operation, unsigned short data);
void cret(unsigned char operation, unsigned short data);
void rst(unsigned char operation, unsigned short data);
void pchl(unsigned char operation, unsigned short data);
void push(unsigned char operation, unsigned short data);
void pushp(unsigned char operation, unsigned short data);
void pop(unsigned char operation, unsigned short data);
void popp(unsigned char operation, unsigned short data);
void xthl(unsigned char operation, unsigned short data);
void sphl(unsigned char operation, unsigned short data);
void in(unsigned char operation, unsigned short data);

bool not_zero(void);
bool zero(void);
bool no_carry(void);
bool carry(void);
bool parity_odd(void);
bool parity_even(void);
bool plus(void);
bool minus(void);

int main(void)
{
	
}

void initialize(void)
{
	condition_checks[0] = &not_zero;
	condition_checks[1] = &zero;
	condition_checks[2] = &no_carry;
	condition_checks[3] = &carry;
	condition_checks[4] = &parity_odd;
	condition_checks[5] = &parity_even;
	condition_checks[6] = &plus;
	condition_checks[7] = &minus;

	register_pairs[B-C] = (B << 3) + C;
	register_pairs[D-E] = (D << 3) + E;
	register_pairs[H-L] = (H << 3) + L;
	register_pairs[S-P] = (S << 3) + P;

	// these include some instructions that shouldn't be mov but they'll get overwritten later
	// in the function
	for (int i = 64; i < 128; i++)
	{
		instructions[i] = &mov;
	}
	for (int i = 6; i < 63; i++)
	{
		instructions[i] = &mov;
	}

	for (int i = 1; i <= 49; i += 16)
	{
		instructions[i] = &lxi;
	}

	instructions[58] = &lda;
	instructions[50] = &sta;
	instructions[42] = &lhld;
	instructions[10 + (B-C << 4)] = &ldax;
	instructions[10 + (D-E << 4)] = &ldax;
	instructions[2 + (B-C << 4)] = &stax;
	instructions[2 + (D-E << 4)] = &stax;
	instructions[235] = &xchg;

	for (int i = 128; i < 136; i++)
	{
		instructions[i] = &add;
	}

	instructions[198] = &adi;

	for (int i = 136; i < 144; i++)
	{
		instructions[i] = &adc;
	}

	instructions[206] = &aci;

	for (int i = 144; i < 152; i++)
	{
		instructions[i] = &sub;
	}

	instructions[214] = &sui;

	for (int i = 152 i < 160; i++)
	{
		instructions[i] = &sbb;
	}

	instructions[222] = &sbi;

	for (int i = 4; i <= 60; i += 8)
	{
		instructions[i] = &inr;
		instructions[i + 1] = &dcr;
	}

	for (int i = 3; i <= 51; i += 16)
	{
		instructions[i] = &inx;
		instructions[i + 8] = &dcx;
	}

	for (int i = 9; i <= 57; i += 16)
	{
		instructions[i] = &dad;
	}

	instructions[39] = &daa;

	for (int i = 160; i <= 167; i++)
	{
		instructions[i] = &ana;
	}

	instructions[230] = &ani;

	for (int i = 168; i <= 175; i++)
	{
		instructions[i] = &xra;
	}

	instructions[238] = &xri;

	for (int i = 176; i <= 183; i++)
	{
		instructions[i] = &ora;
	}

	instructions[246] = &ori;

	for (int i = 184; i <= 191; i++)
	{
		instructions[i] = &cmp;
	}

	instructions[254] = &cpi;
	instructions[7] = &rlc;
	instructions[15] = &rrc;
	instructions[23] = &ral;
	instructions[31] = &rar;
	instructions[47] = &cma;
	instructions[63] = &cmc;
	instructions[55] = &stc;
	instructions[195] = &jmp;
	
	for (int i = 194; i <= 250; i += 8)
	{
		instructions[i] = &jcon;
	}

	instructions[205] = &call;
	
	for (int i = 193; i<= 249; i += 8)
	{
		instructions[i] = &ccall;
	}

	instructions[201] = &ret;

	for (int i = 192; i <= 248; i += 8)
	{
		instructions[i] = &cret;
	}

	for (int i = 199; i <= 255; i += 8)
	{
		instructions[i] = &rst;
	}

	instructions[233] = &pchl;

	for (int i = 197; i <= 229; i += 16)
	{
		instructions[i] = &push;
	}

	instructions[245] = &pushp;

	for (int i = 193; i <= 225; i += 16)
	{
		instructions[i] = &pop;
	}

	instructions[241] = &popp;
	instructions[227] = &xthl;
	instructions[249] = &sphl;
}

void update_flags(unsigned char prev, unsigned char post, bool add)
{
	flags = 0;

	if (post == 0)
	{
		flags |= FLAG_Z;
	}
	if (post & 128)
	{
		flags |= FLAG_S;
	}
	if (post & 2 == 0)
	{
		flags |= FLAG_P;
	}
	if (prev & 128 && !(post & 128))
	{
		flags |= FLAG_C;
	}

	if (add)
	{
		unsigned char diff = post - prev;
		if ((prev & 15) + (diff & 15) < (prev & 15))
		{
			flags |= FLAG_A;
		}
	}
	else
	{
		unsigned char diff = prev - post;
		if ((prev & 15) - (diff & 15) > (prev & 15))
		{
			flags |= FLAG_A;
		}
	}
}

unsigned short get_rp(unsigned char rp_index)
{
	unsigned short r = 0;
	r += registers[(register_pairs[rp_index] & THREE_TO_FIVE_BITS) >> 3] << 8;
	r += registers[(register_pairs[rp_index] & ZERO_TO_TWO_BITS)];
	return r;
}

void set_rp(unsigned char rp_index, unsigned short val)
{
	registers[(register_pairs[rp_index] & THREE_TO_FIVE_BITS) >> 3] = val >> 8;
	registers[(register_pairs[rp_index] & ZERO_TO_TWO_BITS)] = val;
}

bool not_zero(void)
{
	return !(flag & FLAG_Z);
}

bool zero(void)
{
	return flag & FLAG_Z;
}

bool no_carry(void)
{
	return !(flag & FLAG_C);
}

bool carry(void)
{
	return flag & FLAG_C;
}

bool parity_odd(void)
{
	return !(flag & FLAG_P);
}

bool parity_even(void)
{
	return flag & FLAG_P;
}

bool plus(void)
{
	return !(flag & FLAG_S);
}

bool minus(void)
{
	return flag & FLAG_S;
}

void mov(unsigned char operation, unsigned short data)
{
	if (operation == 118) // HLT
	{
		return;
	}

	unsigned char src;
	unsigned char dest = (operation & THREE_TO_FIVE_BITS) >> 3;;

	if (operation & (1 << 6))
	{
		src = operation & ZERO_TO_TWO_BITS;

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
	unsigned short val = (data >> 8) + ((data & 255) << 8);
	set_rp((operation & 48) >> 4, val);
}

void lda(unsigned char operation, unsigned short data)
{
	registers[A] = mem[(data << 8) + (data >> 8)];
}

void sta(unsigned char operation, unsigned short data)
{
	mem[(data << 8) + (data >> 8)] = registers[A];
}

void lhld(unsigned char operation, unsigned short data)
{
	registers[L] = mem[(data << 8) + (data >> 8)];
	registers[H] = mem[(data << 8) + (data >> 8) + 1];
}

void shld(unsigned char operation, unsigned short data)
{
	mem[(data << 8) + (data >> 8)] = registers[L];
	mem[(data << 8) + (data >> 8) + 1] = registers[H];
}

void ldax(unsigned char operation, unsigned short data)
{
	registers[A] = mem[get_rp((operation & 48) >> 4)];
}

void stax(unsigned char operation, unsigned short data)
{
	mem[get_rp((operation & 48) >> 4)] = registers[A];
}

void xchg(unsigned char operation, unsigned short data)
{
	unsigned char tmp = registers[H];
	registers[H] = registers[D];
	registers[D] = tmp;

	tmp = registers[L];
	registers[L] = registers[E];
	registers[E] = tmp;
}

void add(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];

	if (operation & ZERO_TO_TWO_BITS == 6)
	{
		registers[A] += mem[(registers[H] << 8) + registers[L]];
	}
	else
	{
		registers[A] += registers[operation & ZERO_TO_TWO_BITS];
	}

	update_flags(store, registers[A], true);
}

void adi(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];
	registers[A] += data >> 8;
	update_flags(store, registers[A], true);
}

void adc(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];

	if (operation & ZERO_TO_TWO_BITS == 6)
	{
		registers[A] += mem[(registers[H] << 8) + registers[L]] + (flags & FLAG_C);
	}
	else
	{
		registers[A] += registers[operation & ZERO_TO_TWO_BITS] + (flags & FLAG_C);
	}

	update_flags(store, registers[A], true);
}

void aci(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];
	registers[A] += (data >> 8) + (flags & FLAG_C);
	update_flags(store, registers[A], true);
}

void sub(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];

	if (operation & ZERO_TO_TWO_BITS == 6)
	{
		registers[A] -= mem[(registers[H] << 8) + registers[L]];
	}
	else
	{
		registers[A] -= registers[operation & ZERO_TO_TWO_BITS];
	}

	update_flags(store, registers[A], false);
}

void sui(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];
	registers[A] -= data >> 8;
	update_flags(store, registers[A], false);
}

void sbb(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];

	if (operation & ZERO_TO_TWO_BITS == 6)
	{
		registers[A] -= mem[(registers[H] << 8) + registers[L]] + FLAG_C;
	}
	else
	{
		registers[A] -= registers[operation & ZERO_TO_TWO_BITS] + FLAG_C:
	}

	update_flags(store, registers[A], false);
}

void sbi(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];
	registers[A] -= (data >> 8) + FLAG_C;
	update_flags(store, registers[A], false);
}

void inr(unsigned char operation, unsigned short data)
{
	unsigned char store = flags & FLAG_C;
	
	if ((operation & THREE_TO_FIVE_BITS) >> 3 == 6)
	{
		mem[(registers[H] << 8) + registers[L]]++;
		update_flags(mem[(registers[H] << 8) + registers[L]] - 1, mem[(registers[H] << 8) + registers[L]], true);
	}
	else
	{
		registers[(operation & THREE_TO_FIVE_BITS) >> 3]++;
		update_flags(registers[(operation & THREE_TO_FIVE_BITS) >> 3] - 1, registers[(operation & THREE_TO_FIVE_BITS) >> 3], true);
	}

	flags |= store;
}

void dcr(unsigned char operation, unsigned short data)
{
	unsigned char store = flags & FLAG_C;

	if ((operation & THREE_TO_FIVE_BITS) >> 3 == 6)
	{
		mem[(registers[H] << 8) + registers[L]]--;
		update_flags(mem[(registers[H] << 8) + registers[L]] + 1, mem[(registers[H] << 8) + registers[L]], false);
	}
	else
	{
		registers[(operation & THREE_TO_FIVE_BITS) >> 3]--;
		update_flags(registers[(operation & THREE_TO_FIVE_BITS) >> 3] + 1, registers[(operation & THREE_TO_FIVE_BITS) >> 3], false);
	}

	flags |= store;
}

void inx(unsigned char operation, unsigned short data)
{
	unsigned short val = get_rp((operation & 48) >> 4);
	val++;
	set_rp((operation & 48) >> 4, val);
}

void dcx(unsigned char operation, unsigned short data)
{
	unsigned short val = get_rp((operation & 48) >> 4);
	val--;
	set_rp((operation & 48) >> 4, val);
}

void dad(unsigned char operation, unsigned short data)
{
	unsigned short start_value = get_rp(H-L);
	unsigned short end_value = start_value + get_rp((operation & 48) >> 4);
	set_rp(H-L, end_value);
	if (start_value > end_value)
	{
		flags |= FLAG_C;
	}
}

void daa(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];

	if (registers[A] & 15 > 9 || flags & FLAG_A)
	{
		registers[A] += 6;
	}
	if ((registers[A] >> 4) & 15 > 9 || flags & FLAG_C)
	{
		registers[A] += 96;
	}

	update_flags(store, registers[A], true);
}

void ana(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];

	if (operation == 166)
	{
		registers[A] &= mem[get_rp(H-L)];
	}
	else
	{
		registers[A] &= registers[operation & 7];
	}

	update_flags(store, registers[A], true);
	flags & (255 - FLAG_C) | FLAG_A;
}

void ani(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];
	registers[A] &= data >> 8;
	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
}

void xra(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];

	if (operation == 174)
	{
		registers[A] ^= mem[get_rp(H-L)];
	}
	else
	{
		registers[A] ^= registers[operation & 7];
	}

	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
}

void xri(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];
	registers[A] ^= data >> 8;
	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
}

void ora(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];

	if (operation == 182)
	{
		registers[A] |= mem[get_rp(H-L)];
	}
	else
	{
		registers[A] |= registers[operation & 7];
	}

	update_flags(store, registers[A], true);
	flags = flags - & (255 - FLAG_C - FLAG_A);
}

void ori(unsigned char operation, unsigned short data)
{
	unsigned char store = registers[A];
	registers[A] |= data >> 8;
	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
}

void cmp(unsigned char operation, unsigned short data)
{
	unsigned char subtractor;
	if (operation == 190)
	{
		subtractor = mem[get_rp(H-L)];
	}
	else
	{
		subtractor = registers[operation & 7];
	}
	update_flags(registers[A], registers[A] - subtractor);
}

void cpi(unsigned char operation, unsigned short data)
{
	update_flags(registers[A], registers[A] - (data >> 8));
}

void rlc(unsigned char operation, unsigned short data)
{
	unsigned char num = registers[A] >> 7;
	registers[A] <<= 1;
	registers[A] |= num;
	if (num)
	{
		flags |= FLAG_C;
	}
	else
	{
		flags &= 255 - FLAG_C;
	}
}

void rrc(unsigned char operation, unsigned short data)
{
	unsigned char num = registers[A] << 7;
	regsters[A] >>= 1;
	registers[A] |= num;
	if (num)
	{
		flags |= FLAG_C;
	}
	else
	{
		flags &= 255 - FLAG_C;
	}
}

void ral(unsigned char operation, unsigned short data)
{
	unsigned char store = 0;
	if (flags & FLAG_C)
	{
		store = 1;
	}

	if (registers[A] & 127)
	{
		flags |= FLAG_C;
	}
	else
	{
		flags &= 255 - FLAG_C;
	}

	registers[A] <<= 1;
	registers[A] = store | (registers[A] & 1);

void rar(unsigned char operation, unsigned short data)
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
	registers[A] = store | (regsiters[A] & 127);
}

void cma(unsigned char operation, unsigned short data)
{
	registers[A] = ~registers[A];
}

void cmc(unsigned char operation, unsigned short data)
{
	flags ^= FLAG_C;
}

void stc(unsigned char operation, unsigned short data)
{
	flags |= FLAG_C;
}

void jmp(unsigned char operation, unsigned short data)
{
	IP = (data << 8) + (data >> 8);
}

void jcon(unsigned char operation, unsigned short data)
{
	if (condition_checks[(operation & 56) >> 3])
	{
		IP = (data << 8) + (data >> 8);
	}
}

void call(unsigned char operation, unsigned short data)
{
	mem[SP - 1] = IP >> 8;
	mem[SP - 2] = IP & 255;
	SP -= 2;
	IP = (data << 8) + (data >> 8);
}

void ccall(unsigned char operation, unsigned short data)
{
	if (condition_checks[(operation & 56) >> 3])
	{
		mem[SP - 1] = IP >> 8;
		mem[SP - 2] = IP & 255;
		SP -= 2;
		IP = (data << 8) + (data >> 8);
	}
}

void ret(unsigned char operation, unsigned short data)
{
	IP = 0;
	IP += mem[SP] + (mem[SP + 1] << 8);
	SP += 2;
}

void cret(unsigned char operation, unsigned short data)
{
	if (condition_checks[(operation & 56) >> 3])
	{
		IP = 0;
		IP += mem[SP] + (mem[SP + 1] << 8);
		SP += 2;
	}
}

void rst(unsigned char operation, unsigned short data)
{
	mem[SP - 1] = IP >> 8;
	mem[SP - 2] = IP & 255;
	SP -= 2;
	IP = operation & 56;
}

void pchl(unsigned char operation, unsigned short data)
{
	IP = H << 8;
	IP += L;
}

void push(unsigned char operation, unsigned short data)
{
	mem[SP - 1] = registers[register_pairs[(operation & 48) >> 4] >> 3];
	mem[SP - 2] = registers[register_pairs[(operation & 48) >> 4] & 7];
	SP -= 2;
}

void pushp(unsigned char operation, unsigned short data)
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
}

void pop(unsigned char operation, unsigned short data)
{
	registers[register_pairs[(operation & 48) >> 4] >> 3] = mem[SP - 1];
	registers[register_pairs[(operation & 48) >> 4] & 7] = mem[SP - 2];
	SP += 2;
}

void popp(unsigned char operation, unsigned short data)
{
	flags = 0;
	flags += (mem[SP] & 1) << FLAG_C_BIT;
	flags += (mem[SP] & 4);
	flags += (mem[SP] & 16);
	flags += (mem[SP] & 64) >> 6;
	flags += (mem[SP] & 128) >> 6;
	registers[A] = mem[SP + 1];
	SP += 2;
}

void xthl(unsigned char operation, unsigned short data)
{
	unsigned char temp;
	temp = registers[L];
	registers[L] = mem[SP];
	mem[SP] = temp;
	temp = registers[H];
	registers[H] = mem[SP + 1];
	mem[SP + 1] = temp;
}

void sphl(unsigned char operation, unsigned short data)
{
	SP = (registers[H] << 8) + registers[L];
}

void in(unsigned char operation, unsigned short data)
{

}
