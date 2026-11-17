/* See cjk.h. Blob layout: tools/cjkconv.py. */
#include "cjk.h"
#include <string.h>

static const unsigned char *blob;
static size_t blob_len;
static int n_ranges = -1;

static uint32_t u32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

int cjk_set_blob(const unsigned char *b, size_t len) {
    blob = b ? b : redux_cjk_bin;
    blob_len = b ? len : redux_cjk_bin_len;
    n_ranges = 0;   /* a bad blob gives no glyphs, not the embedded ones */
    if (blob_len < 12 || memcmp(blob, "RDXCJK1\0", 8) != 0) return -1;
    uint32_t n = u32(blob + 8);
    if (n > 64 || blob_len < 12 + (size_t)n * 8) return -1;
    size_t need = 12 + (size_t)n * 8;
    for (uint32_t i = 0; i < n; i++) need += (size_t)u32(blob + 12 + i * 8 + 4) * 32;
    if (blob_len < need) return -1;
    n_ranges = (int)n;
    return n_ranges;
}

const uint8_t *cjk_glyph(uint32_t cp) {
    if (n_ranges < 0 && cjk_set_blob(NULL, 0) < 0) return NULL;
    size_t at = 12 + (size_t)n_ranges * 8;
    for (int i = 0; i < n_ranges; i++) {
        uint32_t first = u32(blob + 12 + i * 8), count = u32(blob + 12 + i * 8 + 4);
        if (cp >= first && cp < first + count) {
            const uint8_t *g = blob + at + (size_t)(cp - first) * 32;
            for (int k = 0; k < 32; k++) if (g[k]) return g;
            return NULL;   /* all zero: Unifont has no 16-wide glyph here */
        }
        at += (size_t)count * 32;
    }
    return NULL;
}
