#pragma once
/**
 * Copyright 2016 Daniel Estevez <daniel@destevez.net>.
 * Released into the public domain (Unlicense).
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Decode a 24-bit Golay codeword.
 *  @param data  in/out: 24-bit word; corrected on success.
 *  @return number of bit errors corrected, or -1 if uncorrectable.
 */
int decode_golay24(uint32_t *data);

#ifdef __cplusplus
}
#endif
