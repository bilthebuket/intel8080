#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#define MEMORY_SIZE 65536
#define NUM_REGISTER_ENCODINGS 8
#define NUM_REGISTER_PAIR_ENCODINGS 3
#define NUM_INSTRUCTIONS 256
#define NUM_CONDITIONS 8
#define NUM_PORTS 7

// a single assembly instruction, where the extern unsigned char is the actual instruction and extern unsigned short
// contains the subsquent data/memory address
typedef void (*instruction)(void);

// checks a condition (zero, not zero, carry, not carry, etc)
typedef bool (*condition_check)(void);

// indexs of different registers in the registers array
extern const int A;
extern const int B;
extern const int C;
extern const int D;
extern const int E;
extern const int H;
extern const int L;
extern const int HL_MEM;

extern const int BC;
extern const int DE;
extern const int HL;

// zero, sign, parity, carry, and auxiliary carry flags
extern const int FLAG_Z; // if the result of an mem[IP] is zero
extern const int FLAG_S; // if the result of an mem[IP] leads to the sign bit (most siginificant bit) being 1
extern const int FLAG_P; // if the reuslt of an mem[IP] is even
extern const int FLAG_C; // if there is a wrap around
extern const int FLAG_A; // if there was a carry out of bit 3 into bit 4

extern const int FLAG_Z_BIT;
extern const int FLAG_S_BIT;
extern const int FLAG_P_BIT;
extern const int FLAG_C_BIT;
extern const int FLAG_A_BIT;

extern const int ZERO_TO_TWO_BITS;
extern const int THREE_TO_FIVE_BITS;

extern const int READY_FOR_READ;
extern const int READY_FOR_WRITE;

// registers
extern unsigned char registers[NUM_REGISTER_ENCODINGS];

// each of the elements of the array are a 8 bit int that looks like this:
// GGHHHLLL
// where GG is a garbage value, HHH is the index in registers of the higher order register, 
// and LLL is the index in registers of the lower order register
extern unsigned char register_pairs[NUM_REGISTER_PAIR_ENCODINGS];

extern unsigned short SP;
extern unsigned short IP;

extern unsigned char flags;

extern unsigned char mem[MEMORY_SIZE];

extern bool can_interrupt;

extern sem_t sems[NUM_PORTS];
extern unsigned char ports[NUM_PORTS];

// bit flag for if a port has been updated
extern unsigned char has_been_updated;
extern pthread_t shift_register_thread;

extern pthread_t emulated_cpu_thread;

extern unsigned short shift_register;

extern instruction instructions[NUM_INSTRUCTIONS];
extern const char* names[NUM_INSTRUCTIONS];

extern condition_check condition_checks[NUM_CONDITIONS];

#endif
