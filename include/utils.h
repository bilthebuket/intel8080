#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

void update_flags(unsigned char prev, unsigned char post, bool add);

unsigned short get_rp(unsigned char rp_index);
void set_rp(unsigned char rp_index, unsigned short val);

bool not_zero(void);
bool zero(void);
bool no_carry(void);
bool carry(void);
bool parity_odd(void);
bool parity_even(void);
bool plus(void);
bool minus(void);

void initialize_arrays(void);

void cycle_sleep(int num_cycles);

#endif
