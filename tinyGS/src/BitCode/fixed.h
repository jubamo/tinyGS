#pragma once
/* CCSDS Reed-Solomon GF(2^8) parameters
 * Phil Karn KA9Q / gr-satellites style
 * May be used under the terms of the GNU Lesser General Public License (LGPL)
 */

#include <stdint.h>
#include <string.h>

typedef uint8_t data_t;

/* CCSDS RS(255,223) over GF(2^8)
 * Generator poly starting at FCR=112, primitive element alpha^11 */
#define NN      255
#define FCR     112
#define PRIM    11
#define NROOTS  32
#define A0      NN       /* index form of zero element */
#define IPRIM   116      /* PRIM^-1 mod NN: 11*116=1276=5*255+1 */

/* CCSDS GF tables declared in ccsds_tables.cpp */
#ifdef __cplusplus
extern "C" {
#endif
extern unsigned char CCSDS_alpha_to[];
extern unsigned char CCSDS_index_of[];
#ifdef __cplusplus
}
#endif

#define ALPHA_TO CCSDS_alpha_to
#define INDEX_OF CCSDS_index_of

/* Modulo NN reduction - inputs must be non-negative */
static inline int MODNN(int x) { return x % NN; }

#ifdef __cplusplus
extern "C" {
#endif
int decode_rs_8(data_t *data, int *eras_pos, int no_eras, int pad);
#ifdef __cplusplus
}
#endif
