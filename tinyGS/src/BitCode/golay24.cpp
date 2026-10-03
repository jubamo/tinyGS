/**
 * Copyright 2016 Daniel Estevez <daniel@destevez.net>.
 * Released into the public domain (Unlicense).
 *
 * Algorithm based on:
 * R.H. Morelos-Zaragoza, The Art of Error Correcting Coding,
 * Wiley, 2002; Section 2.2.3
 */

#include "golay24.h"
#include <stdint.h>

#define N 12

static const uint32_t H[N] = {
    0x8008ed, 0x4001db, 0x2003b5, 0x100769,
    0x80ed1,  0x40da3,  0x20b47,  0x1068f,
    0x8d1d,   0x4a3b,   0x2477,   0x1ffe
};

#define B(i) (H[i] & 0xfff)

int decode_golay24(uint32_t *data)
{
    uint32_t r = *data;
    uint16_t s, q;
    uint32_t e;
    int i;

    /* Step 1: s = H * r */
    s = 0;
    for (i = 0; i < N; i++) {
        s <<= 1;
        s |= (uint16_t)__builtin_parity(H[i] & r);
    }

    /* Step 2: if w(s) <= 3, e = (s, 0) */
    if (__builtin_popcount(s) <= 3) {
        e = (uint32_t)s << N;
        goto step8;
    }

    /* Step 3: if w(s + B[i]) <= 2, e = (s + B[i], e_{i+1}) */
    for (i = 0; i < N; i++) {
        if (__builtin_popcount(s ^ B(i)) <= 2) {
            e = (uint32_t)(s ^ B(i)) << N;
            e |= (uint32_t)1 << (N - i - 1);
            goto step8;
        }
    }

    /* Step 4: compute q = B * s */
    q = 0;
    for (i = 0; i < N; i++) {
        q <<= 1;
        q |= (uint16_t)__builtin_parity(B(i) & s);
    }

    /* Step 5: if w(q) <= 3, e = (0, q) */
    if (__builtin_popcount(q) <= 3) {
        e = q;
        goto step8;
    }

    /* Step 6: if w(q + B[i]) <= 2, e = (e_{i+1}, q + B[i]) */
    for (i = 0; i < N; i++) {
        if (__builtin_popcount(q ^ B(i)) <= 2) {
            e = (uint32_t)1 << (2 * N - i - 1);
            e |= (q ^ B(i));
            goto step8;
        }
    }

    /* Step 7: uncorrectable */
    return -1;

step8:
    *data = r ^ e;
    return __builtin_popcount(e);
}
