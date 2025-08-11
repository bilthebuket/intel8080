#include <stdio.h>

#define MEMORY_SIZE 65536
#define NUM_INSTRUCTIONS 255

// zero, sign, parity, carry, and auxiliary carry flags
#define FLAG_Z (0) // if the result of an operation is zero
#define FLAG_S (1 << 1) // if the result of an operation leads to the sign bit (most siginificant bit) being 1
#define FLAG_P (1 << 2) // if the reuslt of an operation is even
#define FLAG_C (1 << 3) // if there is a wrap around
#define FLAG_A (1 << 4) // if there was a carry out of bit 3 into bit 4

// a single assembly instruction, where the unsigned char is the actual instruction and unsigned short
// contains the subsquent data/memory address
typedef void (*op)(unsigned char, unsigned short);

// registers
unsigned char A;
unsigned char B;
unsigned char C;
unsigned char D;
unsigned char E;
unsigned char H;
unsigned char L;
unsigned short SP;
unsigned short IP;

unsigned char flags;

unsigned char mem[MEMORY_SIZE];

op ops[255];

int main(void)
{
	flags = 0;
}
