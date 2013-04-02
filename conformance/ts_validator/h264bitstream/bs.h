#ifndef _BS_H_
#define _BS_H_        1

//#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

// declarations

typedef struct {
	uint8_t *start;
	uint8_t *p;
	uint8_t *end;
	int bits_left;
} bs_t;

static bs_t *bs_new(uint8_t *buf, size_t size);
static void bs_free(bs_t *b);
static bs_t *bs_clone(bs_t *dest, const bs_t *const src);
static bs_t *bs_init(bs_t *b, uint8_t *buf, size_t size);
static uint32_t bs_byte_aligned(const bs_t * const b);
static int bs_eof(const bs_t * const b);
static int bs_overrun(const bs_t * const b);
static int bs_pos(const bs_t * const b);

static uint32_t bs_peek_u1(bs_t *b);
static uint32_t bs_read_u1(bs_t *b);
static uint32_t bs_read_u(bs_t *b, int n);
static uint32_t bs_read_f(bs_t *b, int n);
static uint32_t bs_read_u8(bs_t *b);
static uint32_t bs_read_ue(bs_t *b);
static int32_t bs_read_se(bs_t *b);

static void bs_write_u1(bs_t *b, uint32_t v);
static void bs_write_u(bs_t *b, int n, uint32_t v);
static void bs_write_f(bs_t *b, int n, uint32_t v);
static void bs_write_u8(bs_t *b, uint32_t v);
static void bs_write_ue(bs_t *b, uint32_t v);
static void bs_write_se(bs_t *b, int32_t v);
static void bs_write_ull(bs_t *b, int n, uint64_t v);

static int bs_read_bytes(bs_t *b, uint8_t *buf, int len);
static int bs_write_bytes(bs_t *b, uint8_t *buf, int len);
static int bs_skip_bytes(bs_t *b, int len);
static uint32_t bs_next_bits(bs_t *b, int nbits);

static void bs_skip_u1(bs_t *b);
static void bs_skip_u(bs_t *b, int n);
static int bs_bytes_left(const bs_t * const b);

static uint32_t bs_read_u16(bs_t *b);
static uint32_t bs_read_u24(bs_t *b);
static uint32_t bs_read_u32(bs_t *b);
static uint64_t bs_read_u64(bs_t *b);
static uint64_t bs_read_ull(bs_t *b, int n);

// impl

// FIXME optimize for performance

static inline bs_t *bs_init(bs_t *b, uint8_t *buf, size_t size) {
	if (!b || !buf || !size)
		return NULL;
	b->start = b->p = buf;
	b->end = &buf[size];
	b->bits_left = 8;
	return b;
}

static inline bs_t *bs_new(uint8_t *buf, size_t size) {
	if (!buf || !size)
		return NULL;
	bs_t *b = (bs_t *) calloc(1, sizeof(bs_t));
	if (b)
		bs_init(b, buf, size);
	return b;
}

/**
 * don't free buf, it's client's responsibility
 */
static inline void bs_free(bs_t *b) {
	if (b)
		free(b);
}

static inline bs_t *bs_clone(bs_t *dest, const bs_t *const src) {
	if (!dest || !src)
		return NULL;
	memcpy(dest, src, sizeof(bs_t));
	return dest;
}

static inline uint32_t bs_byte_aligned(const bs_t * const b) {
	return (b && b->bits_left == 8);
}

static inline int bs_bytes_left(const bs_t * const b) {
	return (b ? b->end - b->p : 0);
}

static inline int bs_eof(const bs_t * const b) {
	return (b && b->p >= b->end);
}

static inline int bs_overrun(const bs_t * const b) {
	return (b && b->p > b->end);
}

static inline int bs_pos(const bs_t * const b) {
	return (b ? (b->p > b->end ? b->end - b->start : b->p - b->start) : 0);
}

static inline uint32_t bs_peek_u1(bs_t *b) {
	if (!b || bs_eof(b))
		return 0;
	return ((b->p)[0] >> (b->bits_left - 1)) & 0x01;
}

static inline uint32_t bs_read_u1(bs_t *b) {
	if (!b || bs_eof(b))
		return 0;

	uint32_t result = ((b->p)[0] >> --(b->bits_left)) & 0x01;
	if (!b->bits_left) {
		b->p++;
		b->bits_left = 8;
	}

	return result;
}

static inline void bs_write_u1(bs_t *b, uint32_t v) {
	if (!b || bs_eof(b))
		return;

	(b->p)[0] &= ~(1 << --(b->bits_left));
	(b->p)[0] |= ((v & 0x01) << b->bits_left);

	if (!b->bits_left) {
		b->p++;
		b->bits_left = 8;
	}
}

static inline uint32_t bs_read_u(bs_t *b, int n) {
	if (!b || bs_eof(b))
		return 0;

	uint32_t result = 0;
	while (n--) {
		result <<= 1;
		result |= bs_read_u1(b);
	}
	return result;
}

static inline void bs_write_u(bs_t *b, int n, uint32_t v) {
	if (!b || bs_eof(b))
		return;

	while (n--)
		bs_write_u1(b, (v >> n) & 0x1);
}

static inline uint32_t bs_read_f(bs_t *b, int n) {
	return bs_read_u(b, n);
}

static inline void bs_write_f(bs_t *b, int n, uint32_t v) {
	bs_write_u(b, n, v);
}

static inline uint32_t bs_read_u8(bs_t *b) {
	if (!b || bs_eof(b))
		return 0;

	uint32_t result = 0;
	if (b->bits_left == 8) { // we're in luck, we're aligned
		result = (b->p)[0];
		b->p++;
	} else
		result = bs_read_u(b, 8);

	return result;
}

static inline void bs_write_u8(bs_t *b, uint32_t v) {
	if (!b || bs_eof(b))
		return;

	if (b->bits_left == 8) {
		(b->p)[0] = v & 0xFF;
		b->p++;
	} else
		bs_write_u(b, 8, v);
}

static inline uint32_t bs_read_ue(bs_t *b) {
	if (!b)
		return 0;

	int i = 0;
	while ((bs_read_u1(b) == 0) && (i < 32) && !bs_eof(b))
		i++;

	int32_t result = bs_read_u(b, i);
	result += (1 << i) - 1;
	return result;
}

static inline void bs_write_ue(bs_t *b, uint32_t v) {
	if (!b)
		return;
	if (v == 0)	// FIXME impl
		return;
}

static inline int32_t bs_read_se(bs_t *b) {
	if (!b)
		return 0;

	int32_t result = bs_read_ue(b);
	if (result & 1)
		result = (result + 1) >> 1;
	else
		result = -(result >> 1);

	return result;
}

static inline void bs_write_se(bs_t *b, int32_t v) {
	if (!b)
		return;

	if (v <= 0)
		bs_write_ue(b, - (v << 1));
	else
		bs_write_ue(b, (v << 1) - 1);
}

static inline int _bs_read_write_bytes(bs_t *b, uint8_t *buf, int len, int to_read) {
	if (!b || !buf)
		return 0;
	int bytes_left = b->end - b->p;
	int bytes2copy = (len <= bytes_left ? len : bytes_left);

	uint8_t *src = NULL, *dst = NULL;
	if (to_read) {
		src = b->p;
		dst = buf;
	} else {
		src = buf;
		dst = b->p;
	}

	memcpy(dst, src, bytes2copy);
	b->p += bytes2copy;
	return bytes2copy;
}

static inline int bs_read_bytes(bs_t *b, uint8_t *buf, int len) {
	return _bs_read_write_bytes(b, buf, len, TRUE);
}

static inline int bs_write_bytes(bs_t *b, uint8_t *buf, int len) {
	return _bs_read_write_bytes(b, buf, len, FALSE);
}

static inline int bs_skip_bytes(bs_t *b, int len) {
	if (!b)
		return 0;
	int bytes_left = b->end - b->p;
	int bytes2skip = (len <= bytes_left ? len : bytes_left);
	b->p += bytes2skip;
	return bytes2skip;
}

static inline uint32_t bs_next_bits(bs_t *b, int nbits) {
	if (!b)
		return 0;
	bs_t tmp;
	bs_clone(&tmp, b);
	return bs_read_u(&tmp, nbits);
}

static inline void bs_skip_u1(bs_t *b) {
	if (!b || bs_eof(b))
		return;

	b->bits_left--;
	if (b->bits_left == 0) {
		b->p++;
		b->bits_left = 8;
	}
}

static inline void bs_skip_u(bs_t *b, int n) {
	if (!b || bs_eof(b))
		return;

	while (n--)
		bs_skip_u1(b);
}

static inline uint64_t bs_read_ull(bs_t *b, int n) {
	if (!b || bs_eof(b) || n < 1)
		return 0;

	uint64_t r = 0;
	while (n--)
		r |= ((uint64_t) bs_read_u1(b) << n);
	return r;
}

static inline void bs_write_ull(bs_t *b, int n, uint64_t v) {
	if (!b || bs_eof(b) || n < 1)
		return;

	while (n--)
		bs_write_u1(b, (uint32_t) (v >> n) & 0x1);
}

static inline uint64_t bs_read_uN(bs_t *b, int n) {
	if (!b || bs_eof(b))
		return 0;

	uint64_t r = 0;
	for (int i=1; i <= n; i++)
		r += bs_read_u8(b) << ((n - i) << 3);
	return r;
}

static inline void bs_write_uN(bs_t *b, int n, uint64_t v) {
	if (!b || bs_eof(b))
		return;

	while (n--)
		bs_write_u8(b, (v >> (n << 3)) & 0xFF);
}

static inline uint32_t bs_read_u16(bs_t *b) {
	return (uint32_t) bs_read_uN(b, 2);
}

static inline uint32_t bs_read_u24(bs_t *b) {
	return (uint32_t) bs_read_uN(b, 3);
}

static inline uint32_t bs_read_u32(bs_t *b) {
	return (uint32_t) bs_read_uN(b, 4);
}

static inline uint64_t bs_read_u64(bs_t *b) {
	return bs_read_uN(b, 8);
}

static inline void bs_write_u16(bs_t *b, uint32_t v) {
	bs_write_uN(b, 2, (uint32_t) v);
}

static inline void bs_write_u24(bs_t *b, uint32_t v) {
	bs_write_uN(b, 3, (uint32_t) v);
}

static inline void bs_write_u32(bs_t *b, uint32_t v) {
	bs_write_uN(b, 4, (uint32_t) v);
}

static inline int bs_write_stuffing_bytes(bs_t *b, int v, int len) {
	if (bs_eof(b) || len == 0)
		return 0;

	int num_ch = (b->end - b->p >= len ? len : b->end - b->p);

	if (len > 0) {
		b->p += len;
		if (num_ch > 0)
			memset(b->p, v, num_ch);
		else
			num_ch = 0;
	}

	return num_ch;
}

#ifdef __cplusplus
}
#endif

#endif // _BS_H_
