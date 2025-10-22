#include "global.h"
extern unsigned char rst_addrs[8];

// indexs of different registers in the registers array
const int A = 7;
const int B = 0;
const int C = 1;
const int D = 2;
const int E = 3;
const int H = 4;
const int L = 5;
const int HL_MEM = 6;

const int BC = 0;
const int DE = 1;
const int HL = 2;

// zero, sign, parity, carry, and auxiliary carry flags
const int FLAG_Z = 1; // if the result of an mem[IP] is zero
const int FLAG_S = 1 << 1; // if the result of an mem[IP] leads to the sign bit (most siginificant bit) being 1
const int FLAG_P = 1 << 2; // if the reuslt of an mem[IP] is even
const int FLAG_C = 1 << 3; // if there is a wrap around
const int FLAG_A = 1 << 4; // if there was a carry out of bit 3 into bit 4

const int FLAG_Z_BIT = 0;
const int FLAG_S_BIT = 1;
const int FLAG_P_BIT = 2;
const int FLAG_C_BIT = 3;
const int FLAG_A_BIT = 4;

const int ZERO_TO_TWO_BITS = 7;
const int THREE_TO_FIVE_BITS = 56;

const int READY_FOR_READ = 0;
const int READY_FOR_WRITE = 1;
const int MUTEX = 2;

// registers
unsigned char registers[NUM_REGISTER_ENCODINGS];

// each of the elements of the array are a 8 bit int that looks like this:
// GGHHHLLL
// where GG is a garbage value, HHH is the index in registers of the higher order register, 
// and LLL is the index in registers of the lower order register
unsigned char register_pairs[NUM_REGISTER_PAIR_ENCODINGS];

unsigned short SP = 0x2400;
unsigned short IP = 0;

unsigned char flags = 0;

unsigned char mem[MEMORY_SIZE];
unsigned char rst_addrs[8];

bool can_interrupt = false;
bool actually_can_interrupt = false;

sem_t sems[NUM_PORTS];
unsigned char ports[NUM_PORTS];

// bit flag for if a port has been updated
unsigned char has_been_updated = 0;
pthread_t shift_register_thread;

pthread_t emulated_cpu_thread;

unsigned short shift_register;

instruction instructions[NUM_INSTRUCTIONS];
const char* names[NUM_INSTRUCTIONS];

condition_check condition_checks[NUM_CONDITIONS];

int num_executions = 0;
int num_cycles_executed = 0;
