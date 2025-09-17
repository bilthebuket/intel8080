#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <fcntl.h>

#define MEMORY_SIZE 65536

#define NUM_REGISTER_ENCODINGS 8
#define NUM_REGISTER_PAIR_ENCODINGS 3

// indexs of different registers in the registers array
#define A 7
#define B 0
#define C 1
#define D 2
#define E 3
#define H 4
#define L 5
#define HL_MEM 6

#define BC 0
#define DE 1
#define HL 2

// zero, sign, parity, carry, and auxiliary carry flags
#define FLAG_Z (1) // if the result of an mem[IP] is zero
#define FLAG_S (1 << 1) // if the result of an mem[IP] leads to the sign bit (most siginificant bit) being 1
#define FLAG_P (1 << 2) // if the reuslt of an mem[IP] is even
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

#define NUM_PORTS 7

// a single assembly instruction, where the unsigned char is the actual instruction and unsigned short
// contains the subsquent data/memory address
typedef void (*instruction)(void);

// checks a condition (zero, not zero, carry, not carry, etc)
typedef bool (*condition_check)(void);

// registers
unsigned char registers[NUM_REGISTER_ENCODINGS];

// each of the elements of the array are a 8 bit int that looks like this:
// GGHHHLLL
// where GG is a garbage value, HHH is the index in registers of the higher order register, 
// and LLL is the index in registers of the lower order register
unsigned char register_pairs[NUM_REGISTER_PAIR_ENCODINGS];

unsigned short SP;
unsigned short IP = 0;

unsigned char flags = 0;

unsigned char mem[MEMORY_SIZE];

bool can_interrupt = true;

sem_t sems[NUM_PORTS];
unsigned char ports[NUM_PORTS];

// bit flag for if a port has been updated
unsigned char has_been_updated = 0;
pthread_t shift_register_thread;

pthread_t emulated_cpu_thread;

unsigned short shift_register;

instruction instructions[NUM_INSTRUCTIONS];

condition_check condition_checks[NUM_CONDITIONS];

/*
 * initialize the array of instructions and register pair array
*/
void initialize(void);

void update_flags(unsigned char prev, unsigned char post, bool add);

unsigned short get_rp(unsigned char rp_index);
void set_rp(unsigned char rp_index, unsigned short val);

void* shift_register_func(void*);
void* emulated_cpu_func(void*);

void mov(void);
void lxi(void);
void lda(void);
void sta(void);
void lhld(void);
void shld(void);
void ldax(void);
void stax(void);
void xchg(void);
void add(void);
void adi(void);
void adc(void);
void aci(void);
void sub(void);
void sui(void);
void sbb(void);
void sbi(void);
void inr(void);
void dcr(void);
void inx(void);
void dcx(void);
void dad(void);
void daa(void);
void ana(void);
void ani(void);
void xra(void);
void xri(void);
void ora(void);
void ori(void);
void cmp(void);
void cpi(void);
void rlc(void);
void rrc(void);
void ral(void);
void rar(void);
void cma(void);
void cmc(void);
void stc(void);
void jmp(void);
void jcon(void);
void call(void);
void ccall(void);
void ret(void);
void cret(void);
void rst(void);
void pchl(void);
void push(void);
void pushp(void);
void pop(void);
void popp(void);
void xthl(void);
void sphl(void);
void in(void);
void out(void);
void ei(void);
void di(void);
void hlt(void);
void nop(void);

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
			if (event.type == SDL_QUIT)
			{
				running = 0;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
					case 'a':
						sem_wait(&sems[1]);
						ports[1] |= 1 << 5;
						sem_post(&sems[1]);
						break;

					case 'd':
						sem_wait(&sems[1]);
						ports[1] |= 1 << 6;
						sem_post(&sems[1]);
						break;

					case ' ':
						sem_wait(&sems[1]);
						ports[1] |= 1 << 4;
						sem_post(&sems[1]);
						break;

					case 'c':
						sem_wait(&sems[1]);
						ports[1] |= 1;
						sem_post(&sems[1]);
						break;
				}
			}
			else if (event.type == SDL_KEYUP)
			{
				switch (event.key.keysym.sym)
				{
					case 'a':
						sem_wait(&sems[1]);
						ports[1] &= (255 ^ (1 << 5));
						sem_post(&sems[1]);
						break;

					case 'd':
						sem_wait(&sems[1]);
						ports[1] &= (255 ^ (1 << 6));
						sem_post(&sems[1]);
						break;

					case ' ':
						sem_wait(&sems[1]);
						ports[1] &= (255 ^ (1 << 4));
						sem_post(&sems[1]);
						break;

					case 'c':
						sem_wait(&sems[1]);
						ports[1] &= 254;
						sem_post(&sems[1]);
						break;
				}
			}
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
	for (int i = 0; i < NUM_PORTS; i++)
	{
		if (sem_init(&sems[i], 0, 1) != 0)
		{
			printf("Could not initalize semaphore\n");
			exit(1);
		}
	}

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

	int fd = open("invaders/invaders.h", O_RDONLY);
	read(fd, mem, 2048);
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

	condition_checks[0] = &not_zero;
	condition_checks[1] = &zero;
	condition_checks[2] = &no_carry;
	condition_checks[3] = &carry;
	condition_checks[4] = &parity_odd;
	condition_checks[5] = &parity_even;
	condition_checks[6] = &plus;
	condition_checks[7] = &minus;

	register_pairs[BC] = (B << 3) + C;
	register_pairs[DE] = (D << 3) + E;
	register_pairs[HL] = (H << 3) + L;

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
	instructions[10 + (BC << 4)] = &ldax;
	instructions[10 + (DE << 4)] = &ldax;
	instructions[2 + (BC << 4)] = &stax;
	instructions[2 + (DE << 4)] = &stax;
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

	for (int i = 152; i < 160; i++)
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
	instructions[219] = &in;
	instructions[211] = &out;
	instructions[251] = &ei;
	instructions[243] = &di;
	instructions[118] = &hlt;
	instructions[0] = &nop;
}

void* emulated_cpu_func(void*)
{
	while (true)
	{
		(*instructions[mem[IP]])();
	}

	return NULL;
}

void* shift_register_func(void*)
{
	unsigned short val = 0;
	while (true)
	{
		sem_wait(&sems[4]);

		if (has_been_updated & (1 << 4))
		{
			val >>= 8;
			val += ports[4] << 8;
			has_been_updated ^= 1 << 4;
		}
		
		sem_post(&sems[4]);

		sem_wait(&sems[2]);
		sem_wait(&sems[3]);

		ports[3] = val >> (8 - (ports[2] & 7));

		sem_post(&sems[2]);
		sem_post(&sems[3]);
	}

	return NULL;
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
	return !(flags & FLAG_Z);
}

bool zero(void)
{
	return flags & FLAG_Z;
}

bool no_carry(void)
{
	return !(flags & FLAG_C);
}

bool carry(void)
{
	return flags & FLAG_C;
}

bool parity_odd(void)
{
	return !(flags & FLAG_P);
}

bool parity_even(void)
{
	return flags & FLAG_P;
}

bool plus(void)
{
	return !(flags & FLAG_S);
}

bool minus(void)
{
	return flags & FLAG_S;
}

void mov(void)
{
	if (mem[IP] == 118) // HLT
	{
		return;
	}

	unsigned char src;
	unsigned char dest = (mem[IP] & THREE_TO_FIVE_BITS) >> 3;;

	if (mem[IP] & (1 << 6))
	{
		src = mem[IP] & ZERO_TO_TWO_BITS;

		if (src == HL_MEM)
		{
			src = mem[registers[H] << 8 + registers[L]];
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
		IP += 2;
	}

	if (dest == HL_MEM)
	{
		mem[registers[H] << 8 + registers[L]] = src;
	}
	else
	{
		registers[dest] = src;
	}
}

void lxi(void)
{
	unsigned short val = mem[IP + 1] + (mem[IP + 2] << 8);
	set_rp((mem[IP] & 48) >> 4, val);
	IP += 3;
}

void lda(void)
{
	registers[A] = mem[(mem[IP + 2] << 8) + mem[IP + 1]];
	IP += 3;
}

void sta(void)
{
	mem[(mem[IP + 2] << 8) + mem[IP + 1]] = registers[A];
	IP += 3;
}

void lhld(void)
{
	registers[L] = mem[(mem[IP + 2] << 8) + (mem[IP + 1])];
	registers[H] = mem[(mem[IP + 2] << 8) + (mem[IP + 1]) + 1];
	IP += 3;
}

void shld(void)
{
	mem[(mem[IP + 2] << 8) + (mem[IP + 1])] = registers[L];
	mem[(mem[IP + 2] << 8) + (mem[IP + 1]) + 1] = registers[H];
	IP += 3;
}

void ldax(void)
{
	registers[A] = mem[get_rp((mem[IP] & 48) >> 4)];
	IP++;
}

void stax(void)
{
	mem[get_rp((mem[IP] & 48) >> 4)] = registers[A];
	IP++;
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
}

void add(void)
{
	unsigned char store = registers[A];

	if (mem[IP] & ZERO_TO_TWO_BITS == 6)
	{
		registers[A] += mem[(registers[H] << 8) + registers[L]];
	}
	else
	{
		registers[A] += registers[mem[IP] & ZERO_TO_TWO_BITS];
	}

	update_flags(store, registers[A], true);
	IP++;
}

void adi(void)
{
	unsigned char store = registers[A];
	registers[A] += mem[IP + 1];
	update_flags(store, registers[A], true);
	IP += 2;
}

void adc(void)
{
	unsigned char store = registers[A];

	if (mem[IP] & ZERO_TO_TWO_BITS == 6)
	{
		registers[A] += mem[(registers[H] << 8) + registers[L]] + (flags & FLAG_C);
	}
	else
	{
		registers[A] += registers[mem[IP] & ZERO_TO_TWO_BITS] + (flags & FLAG_C);
	}

	update_flags(store, registers[A], true);
	IP++;
}

void aci(void)
{
	unsigned char store = registers[A];
	registers[A] += (mem[IP + 1]) + (flags & FLAG_C);
	update_flags(store, registers[A], true);
	IP += 2;
}

void sub(void)
{
	unsigned char store = registers[A];

	if (mem[IP] & ZERO_TO_TWO_BITS == 6)
	{
		registers[A] -= mem[(registers[H] << 8) + registers[L]];
	}
	else
	{
		registers[A] -= registers[mem[IP] & ZERO_TO_TWO_BITS];
	}

	update_flags(store, registers[A], false);
	IP++;
}

void sui(void)
{
	unsigned char store = registers[A];
	registers[A] -= mem[IP + 1];
	update_flags(store, registers[A], false);
	IP += 2;
}

void sbb(void)
{
	unsigned char store = registers[A];

	if (mem[IP] & ZERO_TO_TWO_BITS == 6)
	{
		registers[A] -= mem[(registers[H] << 8) + registers[L]] + FLAG_C;
	}
	else
	{
		registers[A] -= registers[mem[IP] & ZERO_TO_TWO_BITS] + FLAG_C;
	}

	update_flags(store, registers[A], false);
	IP++;
}

void sbi(void)
{
	unsigned char store = registers[A];
	registers[A] -= (mem[IP + 1]) + FLAG_C;
	update_flags(store, registers[A], false);
	IP += 2;
}

void inr(void)
{
	unsigned char store = flags & FLAG_C;
	
	if ((mem[IP] & THREE_TO_FIVE_BITS) >> 3 == 6)
	{
		mem[(registers[H] << 8) + registers[L]]++;
		update_flags(mem[(registers[H] << 8) + registers[L]] - 1, mem[(registers[H] << 8) + registers[L]], true);
	}
	else
	{
		registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3]++;
		update_flags(registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3] - 1, registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3], true);
	}

	flags |= store;
	IP++;
}

void dcr(void)
{
	unsigned char store = flags & FLAG_C;

	if ((mem[IP] & THREE_TO_FIVE_BITS) >> 3 == 6)
	{
		mem[(registers[H] << 8) + registers[L]]--;
		update_flags(mem[(registers[H] << 8) + registers[L]] + 1, mem[(registers[H] << 8) + registers[L]], false);
	}
	else
	{
		registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3]--;
		update_flags(registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3] + 1, registers[(mem[IP] & THREE_TO_FIVE_BITS) >> 3], false);
	}

	flags |= store;
	IP++;
}

void inx(void)
{
	unsigned short val = get_rp((mem[IP] & 48) >> 4);
	val++;
	set_rp((mem[IP] & 48) >> 4, val);
	IP++;
}

void dcx(void)
{
	unsigned short val = get_rp((mem[IP] & 48) >> 4);
	val--;
	set_rp((mem[IP] & 48) >> 4, val);
	IP++;
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
}

void daa(void)
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
	IP++;
}

void ana(void)
{
	unsigned char store = registers[A];

	if (mem[IP] == 166)
	{
		registers[A] &= mem[get_rp(HL)];
	}
	else
	{
		registers[A] &= registers[mem[IP] & 7];
	}

	update_flags(store, registers[A], true);
	flags & (255 - FLAG_C) | FLAG_A;
	IP++;
}

void ani(void)
{
	unsigned char store = registers[A];
	registers[A] &= mem[IP + 1];
	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP += 2;
}

void xra(void)
{
	unsigned char store = registers[A];

	if (mem[IP] == 174)
	{
		registers[A] ^= mem[get_rp(HL)];
	}
	else
	{
		registers[A] ^= registers[mem[IP] & 7];
	}

	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP++;
}

void xri(void)
{
	unsigned char store = registers[A];
	registers[A] ^= mem[IP + 1];
	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP += 2;
}

void ora(void)
{
	unsigned char store = registers[A];

	if (mem[IP] == 182)
	{
		registers[A] |= mem[get_rp(HL)];
	}
	else
	{
		registers[A] |= registers[mem[IP] & 7];
	}

	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP++;
}

void ori(void)
{
	unsigned char store = registers[A];
	registers[A] |= mem[IP + 1];
	update_flags(store, registers[A], true);
	flags = flags & (255 - FLAG_C - FLAG_A);
	IP += 2;
}

void cmp(void)
{
	unsigned char subtractor;
	if (mem[IP] == 190)
	{
		subtractor = mem[get_rp(HL)];
	}
	else
	{
		subtractor = registers[mem[IP] & 7];
	}
	update_flags(registers[A], registers[A] - subtractor, false);
	IP++;
}

void cpi(void)
{
	update_flags(registers[A], registers[A] - (mem[IP + 1]), false);
	IP += 2;
}

void rlc(void)
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
	IP++;
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
}

void ral(void)
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
	IP++;
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
}

void cma(void)
{
	registers[A] = ~registers[A];
	IP++;
}

void cmc(void)
{
	flags ^= FLAG_C;
	IP++;
}

void stc(void)
{
	flags |= FLAG_C;
	IP++;
}

void jmp(void)
{
	IP = (mem[IP + 2] << 8) + (mem[IP + 1]);
}

void jcon(void)
{
	if (condition_checks[(mem[IP] & 56) >> 3])
	{
		IP = (mem[IP + 2] << 8) + (mem[IP + 1]);
	}
	else
	{
		IP += 3;
	}
}

void call(void)
{
	mem[SP - 1] = IP >> 8;
	mem[SP - 2] = IP & 255;
	SP -= 2;
	IP = (mem[IP + 2] << 8) + (mem[IP + 1]);
}

void ccall(void)
{
	if (condition_checks[(mem[IP] & 56) >> 3])
	{
		mem[SP - 1] = IP >> 8;
		mem[SP - 2] = IP & 255;
		SP -= 2;
		IP = (mem[IP + 2] << 8) + (mem[IP + 1]);
	}
	else
	{
		IP += 3;
	}
}

void ret(void)
{
	IP = 0;
	IP += mem[SP] + (mem[SP + 1] << 8);
	SP += 2;
}

void cret(void)
{
	if (condition_checks[(mem[IP] & 56) >> 3])
	{
		IP = 0;
		IP += mem[SP] + (mem[SP + 1] << 8);
		SP += 2;
	}
	else
	{
		IP++;
	}
}

void rst(void)
{
	mem[SP - 1] = IP >> 8;
	mem[SP - 2] = IP & 255;
	SP -= 2;
	IP = mem[IP] & 56;
}

void pchl(void)
{
	IP = H << 8;
	IP += L;
}

void push(void)
{
	mem[SP - 1] = registers[register_pairs[(mem[IP] & 48) >> 4] >> 3];
	mem[SP - 2] = registers[register_pairs[(mem[IP] & 48) >> 4] & 7];
	SP -= 2;
	IP++;
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
}

void pop(void)
{
	registers[register_pairs[(mem[IP] & 48) >> 4] >> 3] = mem[SP - 1];
	registers[register_pairs[(mem[IP] & 48) >> 4] & 7] = mem[SP - 2];
	SP += 2;
	IP++;
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
}

void sphl(void)
{
	SP = (registers[H] << 8) + registers[L];
	IP++;
}

void in(void)
{
	sem_wait(&sems[mem[IP + 1]]);
	registers[A] = ports[mem[IP + 1]];
	sem_post(&sems[mem[IP + 1]]);
	IP += 2;
}

void out(void)
{
	sem_wait(&sems[mem[IP + 1]]);
	ports[mem[IP + 1]] = registers[A];
	has_been_updated |= 1 << 4;
	sem_post(&sems[mem[IP + 1]]);
	IP += 2;
}

void ei(void)
{
	can_interrupt = true;
}

void di(void)
{
	can_interrupt = false;
}

void hlt(void)
{
	printf("Halted\n");
}

void nop(void)
{
	return;
}
