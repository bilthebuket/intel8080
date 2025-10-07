#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "global.h"
#include "instructions.h"

void initialize_arrays(void)
{
	for (int i = 0; i < NUM_PORTS; i++)
	{
		if (sem_init(&sems[i][READY_FOR_WRITE], 0, 1) != 0)
		{
			printf("Could not initialize semaphore\n");
			exit(1);
		}
		if (sem_init(&sems[i][READY_FOR_READ], 0, 0) != 0)
		{
			printf("Could not initialize semaphore.\n");
			exit(1);
		}
	}

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
	
	for (int i = 0; i < 256; i++)
	{
		instructions[i] = &invalid;
	}

	for (int i = 64; i < 128; i++)
	{
		instructions[i] = &mov;
	}
	for (int i = 6; i < 63; i += 8)
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
	instructions[34] = &shld;
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

	int num_on = 0;
	for (int i = 0; i < 8; i++)
	{
		if (post & (1 << i))
		{
			num_on++;
		}
	}
	if (num_on % 2 == 0)
	{
		flags |= FLAG_P;
	}

	if (add)
	{
		for (unsigned char i = 0; i < (unsigned char) (post - prev); i++)
		{
			if ((prev + i) & 128 && !((prev + i + 1) & 128))
			{
				flags |= FLAG_C;
			}
			if ((((prev + i) & 8) && !((prev + i + 1) & 8)) || prev + i == 255)
			{
				flags |= FLAG_A;
			}
		}
	}
	else
	{
		for (unsigned char i = 0; i < (unsigned char) (prev - post); i++)
		{
			if (!((prev - i) & 128) && ((prev - i - 1) & 128))
			{
				flags |= FLAG_C;
			}
			if ((((prev - i) & 16) && !((prev - i - 1) & 16)) || prev - i == 0)
			{
				flags |= FLAG_A;
			}
		}
	}
}

unsigned short get_rp(unsigned char rp_index)
{
	unsigned short r = 0;
	if (rp_index == 3)
	{
		r = SP;
	}
	else
	{
		r += registers[(register_pairs[rp_index] & THREE_TO_FIVE_BITS) >> 3] << 8;
		r += registers[(register_pairs[rp_index] & ZERO_TO_TWO_BITS)];
	}
	return r;
}

void set_rp(unsigned char rp_index, unsigned short val)
{
	if (rp_index == 3)
	{
		SP = val;
	}
	else
	{
		registers[(register_pairs[rp_index] & THREE_TO_FIVE_BITS) >> 3] = val >> 8;
		registers[(register_pairs[rp_index] & ZERO_TO_TWO_BITS)] = val;
	}
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
