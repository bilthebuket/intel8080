#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "global.h"
#include "instructions.h"

void initialize_arrays(void)
{
	for (int i = 0; i < NUM_PORTS; i++)
	{
		if (sem_init(&sems[i], 0, 1) != 0)
		{
			printf("Could not initialize semaphore\n");
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

	for (int i = 0; i < 256; i++)
	{
		instructions[i] = &invalid;
		names[i] = "Invalid";
	}

	for (int i = 64; i < 128; i++)
	{
		instructions[i] = &mov;
		names[i] = "mov r1, r2";
	}
	for (int i = 70; i <= 126; i += 8)
	{
		names[i] = "mov r, M";
	}
	for (int i = 112; i <= 119; i++)
	{
		names[i] = "mov M, r";
	}
	for (int i = 6; i < 63; i += 8)
	{
		instructions[i] = &mov;
		names[i] = "mvi r, data";
	}
	names[0x36] = "mvi M, data";

	for (int i = 1; i <= 49; i += 16)
	{
		instructions[i] = &lxi;
		names[i] = "lxi rp, data 16";
	}

	instructions[58] = &lda;
	names[58] = "lda addr";

	instructions[50] = &sta;
	names[50] = "sta addr";

	instructions[42] = &lhld;
	names[42] = "lhld addr";

	instructions[34] = &shld;
	names[34] = "shld addr";

	instructions[10 + (BC << 4)] = &ldax;
	names[10 + (BC << 4)] = "ldax rp";

	instructions[10 + (DE << 4)] = &ldax;
	names[10 + (DE << 4)] = "ldax rp";

	instructions[2 + (BC << 4)] = &stax;
	names[2 + (BC << 4)] = "stax rp";

	instructions[2 + (DE << 4)] = &stax;
	names[2 + (DE << 4)] = "stax rp";

	instructions[235] = &xchg;
	names[235] = "xchg";

	for (int i = 128; i < 136; i++)
	{
		instructions[i] = &add;
		names[i] = "add r";
	}
	names[0x86] = "add M";

	instructions[198] = &adi;
	names[198] = "adi data";

	for (int i = 136; i < 144; i++)
	{
		instructions[i] = &adc;
		names[i] = "adc r";
	}
	names[0x8E] = "adc M";

	instructions[206] = &aci;
	names[206] = "aci M";

	for (int i = 144; i < 152; i++)
	{
		instructions[i] = &sub;
		names[i] = "sub r";
	}
	names[0x96] = "sub M";

	instructions[214] = &sui;
	names[214] = "sui data";

	for (int i = 152; i < 160; i++)
	{
		instructions[i] = &sbb;
		names[i] = "sbb r";
	}
	names[0x9E] = "sbb M";

	instructions[222] = &sbi;
	names[0xDE] = "sbi data";

	for (int i = 4; i <= 60; i += 8)
	{
		instructions[i] = &inr;
		instructions[i + 1] = &dcr;
		names[i] = "inr r";
		names[i + 1] = "dcr r";
	}
	names[0x34] = "inr M";
	names[0x35] = "dcr M";

	for (int i = 3; i <= 51; i += 16)
	{
		instructions[i] = &inx;
		instructions[i + 8] = &dcx;
		names[i] = "inx rp";
		names[i + 8] = "dcx rp";
	}

	for (int i = 9; i <= 57; i += 16)
	{
		instructions[i] = &dad;
		names[i] = "dad rp";
	}

	instructions[39] = &daa;
	names[39] = "daa";

	for (int i = 160; i <= 167; i++)
	{
		instructions[i] = &ana;
		names[i] = "ana r";
	}
	names[0xA6] = "ana M";

	instructions[230] = &ani;
	names[230] = "ani data";

	for (int i = 168; i <= 175; i++)
	{
		instructions[i] = &xra;
		names[i] = "xra r";
	}
	names[0xAE] = "xra M";

	instructions[238] = &xri;
	names[238] = "xri data";

	for (int i = 176; i <= 183; i++)
	{
		instructions[i] = &ora;
		names[i] = "ora r";
	}
	names[0xB6] = "ora M";

	instructions[246] = &ori;
	names[246] = "ori data";

	for (int i = 184; i <= 191; i++)
	{
		instructions[i] = &cmp;
		names[i] = "cmp r";
	}
	names[0xBE] = "cmp M";

	instructions[254] = &cpi;
	names[254] = "cpi data";

	instructions[7] = &rlc;
	names[7] = "rlc";

	instructions[15] = &rrc;
	names[15] = "rrc";

	instructions[23] = &ral;
	names[23] = "ral";

	instructions[31] = &rar;
	names[31] = "rar";

	instructions[47] = &cma;
	names[47] = "cma";

	instructions[63] = &cmc;
	names[63] = "cmc";

	instructions[55] = &stc;
	names[55] = "stc";

	instructions[195] = &jmp;
	names[195] = "jmp addr";
	
	for (int i = 194; i <= 250; i += 8)
	{
		instructions[i] = &jcon;
		names[i] = "jcon addr";
	}

	instructions[205] = &call;
	names[205] = "call addr";
	
	for (int i = 196; i<= 252; i += 8)
	{
		instructions[i] = &ccall;
		names[i] = "ccall addr";
	}

	instructions[201] = &ret;
	names[201] = "ret";

	for (int i = 192; i <= 248; i += 8)
	{
		instructions[i] = &cret;
		names[i] = "cret";
	}

	for (int i = 199; i <= 255; i += 8)
	{
		instructions[i] = &rst;
		names[i] = "rst n";
	}

	instructions[233] = &pchl;
	names[233] = "pchl";

	for (int i = 197; i <= 229; i += 16)
	{
		instructions[i] = &push;
		names[i] = "push rp";
	}

	instructions[245] = &pushp;
	names[245] = "push psw";

	for (int i = 193; i <= 225; i += 16)
	{
		instructions[i] = &pop;
		names[i] = "pop rp";
	}

	instructions[241] = &popp;
	names[241] = "pop psw";

	instructions[227] = &xthl;
	names[227] = "xthl";

	instructions[249] = &sphl;
	names[249] = "sphl";

	instructions[219] = &in;
	names[219] = "in port";

	instructions[211] = &out;
	names[211] = "out port";

	instructions[251] = &ei;
	names[251] = "ei";

	instructions[243] = &di;
	names[243] = "di";

	instructions[118] = &hlt;
	names[118] = "hlt";

	instructions[0] = &nop;
	names[0] = "nop";
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
