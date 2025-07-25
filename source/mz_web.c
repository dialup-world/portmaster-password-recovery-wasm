/* 
 * Modification of mz.c so it can be compiled by Emscripten
 * and the resulting .wasm file can run in a browser. The
 * original messages from mz.c are preserved below.
 *
 * 2025-07-16 Mike Dank / Famicoman
 */

/*
 * This code implements a RESPONSE generator for CHALLENGES
 * originated trought the password override procedure of
 * Livingston Remote Access equipment. Namely, it was tested
 * with PortMaster3 known-good CHALLENGE/RESPONSE pairs, and
 * hands-on on a PortMaster3 unit. It's hoped to work on all
 * ranges of equipment that implement the same procedure for
 * password override.
 *
 * The RESPONSE is just an MD5 digest (converted to a limited
 * range of ascii chars) of a message composed from the
 * CHALLENGE (16 bytes, all in the same ascii range) and from
 * 40 extra bytes.
 *
 * If you use this code, in whatever way, I would rather like
 * you to keep some reference to it's chosen name (mz). That is
 * not, however, a requirement. Like the MD5 code for which I'm
 * grateful, this code is in the public domain; do with it what
 * you wish.
 *
 * August, 2001 - pmsac@toxyn.org
 */

/* The following is the original message for the MD5 code
 * with a little add-on from me */

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 *
 * Changed so as no longer to depend on Colin Plumb's `usual.h' header
 * definitions; now uses stuff from dpkg's config.h.
 *  - Ian Jackson <ijackson@nyx.cs.du.edu>.
 * Still in the public domain.
 *
 * Modularised for Nemesis by Stephen Early
 *
 * Got the code from Nemesis, but don't need the extras, so I cleaned up
 * a bit.
 * pmsac@toxyn.org
 */

#include <emscripten/emscripten.h>
#include <string.h>
#include <stdio.h>

#include <stdint.h>

struct MD5Context {
    uint32_t buf[4];
    uint32_t bytes[2];
    uint32_t in[16];
};

void MD5Init(struct MD5Context *ctx);
void MD5Update(struct MD5Context *ctx, const uint8_t *buf, unsigned len);
void MD5Final(uint8_t digest[16], struct MD5Context *ctx);
void MD5Transform(uint32_t buf[4], const uint32_t in[16]);

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))
#define MD5STEP(f, w, x, y, z, in, s) \
    (w += f(x, y, z) + in, w = (w << s | w >> (32 - s)) + x)

void MD5Init(struct MD5Context *ctx) {
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;
    ctx->bytes[0] = 0;
    ctx->bytes[1] = 0;
}

void byteSwap(uint32_t *buf, unsigned words) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    uint8_t *p = (uint8_t *)buf;
    do {
        *buf++ = (uint32_t)((unsigned)p[3] << 8 | p[2]) << 16 |
                 ((unsigned)p[1] << 8 | p[0]);
        p += 4;
    } while (--words);
#endif
}

void MD5Update(struct MD5Context *ctx, const uint8_t *buf, unsigned len) {
    uint32_t t = ctx->bytes[0];
    if ((ctx->bytes[0] = t + len) < t)
        ctx->bytes[1]++;
    t = 64 - (t & 0x3f);
    if (t > len) {
        memcpy((uint8_t *)ctx->in + 64 - t, buf, len);
        return;
    }
    memcpy((uint8_t *)ctx->in + 64 - t, buf, t);
    byteSwap(ctx->in, 16);
    MD5Transform(ctx->buf, ctx->in);
    buf += t;
    len -= t;
    while (len >= 64) {
        memcpy(ctx->in, buf, 64);
        byteSwap(ctx->in, 16);
        MD5Transform(ctx->buf, ctx->in);
        buf += 64;
        len -= 64;
    }
    memcpy(ctx->in, buf, len);
}

void MD5Final(uint8_t digest[16], struct MD5Context *ctx) {
    int count = ctx->bytes[0] & 0x3f;
    uint8_t *p = (uint8_t *)ctx->in + count;
    *p++ = 0x80;
    count = 56 - 1 - count;
    if (count < 0) {
        memset(p, 0, count + 8);
        byteSwap(ctx->in, 16);
        MD5Transform(ctx->buf, ctx->in);
        p = (uint8_t *)ctx->in;
        count = 56;
    }
    memset(p, 0, count);
    byteSwap(ctx->in, 14);
    ctx->in[14] = ctx->bytes[0] << 3;
    ctx->in[15] = ctx->bytes[1] << 3 | ctx->bytes[0] >> 29;
    MD5Transform(ctx->buf, ctx->in);
    byteSwap(ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    memset(ctx, 0, sizeof(*ctx));
}

void MD5Transform(uint32_t buf[4], const uint32_t in[16]) {
    uint32_t a = buf[0], b = buf[1], c = buf[2], d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

#define DIGLEN 16
#define BUFLEN 56

EMSCRIPTEN_KEEPALIVE
void get_response(const char *input, char *output) {
    struct MD5Context ctx;
    unsigned char digest[DIGLEN];
    char buf[BUFLEN] = {
        0
    };
    memcpy(buf, input, DIGLEN);
    memcpy(buf + DIGLEN,
           "rror: Host Address must be in the format",
           BUFLEN - DIGLEN);

    MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char *)buf, BUFLEN);
    MD5Final(digest, &ctx);

    for (int i = 0; i < DIGLEN; ++i) {
        output[i] = digest[i] % 0x2b + 0x30;
    }
    output[DIGLEN] = '\0';
}
