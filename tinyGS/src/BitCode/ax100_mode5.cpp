/*
 * AX.100 Mode 5 decoder
 *
 * Frame layout (after sync word):
 *   [0..2]   : 24-bit Golay codeword encoding the 8-bit frame_len
 *   [3..257] : CCSDS-randomised RS(255,223) block, padded to 255 bytes
 *
 * Decode steps:
 *   1. Golay-decode bytes 0-2 to recover frame_len
 *   2. CCSDS de-randomise bytes 3 .. (3 + RS_LEN - 1)
 *   3. RS-decode the block (pad = RS_LEN - frame_len)
 *   4. Payload = first (frame_len - 32) bytes of corrected block
 *
 * Based on gr-satellites by Daniel Estevez EA4GPZ.
 * Modified for tinyGS by Stefan/OE6ISP.
 */

#include "ax100_mode5.h"
#include "fixed.h"
#include "randomizer.h"
#include "golay24.h"

#include <stdint.h>
#include <string.h>

int ax100_mode5_decode(const uint8_t *in, int in_len,
                       uint8_t *out, int *out_len,
                       ax100_mode5_info_t *info)
{
    uint8_t data[AX100_M5_HEADER_LEN + AX100_M5_RS_LEN];
    uint8_t scratch[AX100_M5_RS_LEN];
    char    seq[AX100_M5_RS_LEN];

    if (!in || !out || !out_len) return -1;
    if (in_len < AX100_M5_HEADER_LEN + 1) return -2;

    memset(data, 0, sizeof(data));
    memset(scratch, 0, sizeof(scratch));

    if (in_len > (int)sizeof(data))
        in_len = (int)sizeof(data);

    memcpy(data, in, in_len);

    /* Step 1: Golay-decode the 24-bit header */
    uint32_t length_field =
        ((uint32_t)data[0] << 16) |
        ((uint32_t)data[1] <<  8) |
        ((uint32_t)data[2]);

    int golay_res = decode_golay24(&length_field);
    if (golay_res < 0)
        return -3;

    int frame_len = (int)(length_field & 0xff);

    if (in_len < AX100_M5_HEADER_LEN + frame_len)
        return -6;

    if (frame_len <= 32 || frame_len > AX100_M5_RS_LEN)
        return -4;

    /* Step 2: CCSDS de-randomise the RS block */
    ccsds_generate_sequence(seq, AX100_M5_RS_LEN);
    ccsds_xor_sequence(data + AX100_M5_HEADER_LEN, seq, AX100_M5_RS_LEN);

    /* Step 3: RS decode */
    memcpy(scratch, data + AX100_M5_HEADER_LEN, frame_len);

    int pad    = AX100_M5_RS_LEN - frame_len;
    int rs_res = decode_rs_8(scratch, NULL, 0, pad);

    if (rs_res < 0)
        return -5;

    /* Step 4: extract payload */
    int payload_len = frame_len - 32;   /* 32 RS parity bytes */
    memcpy(out, scratch, payload_len);
    *out_len = payload_len;

    if (info) {
        info->golay_errors = golay_res;
        info->rs_errors    = rs_res;
        info->frame_len    = frame_len;
    }

    return 0;
}
