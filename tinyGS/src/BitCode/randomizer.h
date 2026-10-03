#pragma once
/*
 * Copyright (c) 2008 Johan Christiansen
 * Copyright (c) 2012 Jeppe Ledet-Pedersen <jlp@satlab.org>
 * MIT License
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Generate CCSDS pseudo-random sequence (polynomial x^8+x^7+x^5+x^3+1). */
void ccsds_generate_sequence(char *sequence, int length);

/** XOR data with pre-generated CCSDS sequence. */
void ccsds_xor_sequence(unsigned char *data, char *sequence, int length);

#ifdef __cplusplus
}
#endif
