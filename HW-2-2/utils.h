#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

//There's 8 bits in a byte
static const int bitsperbyte=8;

/*
 * Uses mmap to allocate size bytes of memory, and returns a pointer to the allocated memory. The memory is page-aligned, and the size is rounded up to a multiple of the page size.
 *
 * @param size The number of bytes to allocate.
 * @return A pointer to the allocated memory, or mmap message if the allocation fails.
*/
extern void *mmalloc(size_t size);

/**
 * Frees memory previously allocated by mmalloc, with the original size.
 *
 * @param p A pointer to the memory to free.
 * @param size The number of bytes to free.
 */
extern void mmfree(void *p, size_t size);


/**
 * Divides n by d and rounds up to the nearest integer.
 *
 * @param n The numerator.
 * @param d The denominator.
 * @return The result of the division, rounded up to the nearest integer.
*/
extern size_t divup(size_t n, size_t d);

/**
 * Converts a number of bits to the equivalent number of bytes, rounding up to the nearest byte.
 *
 * @param bits The number of bits.
 * @return The equivalent number of bytes, rounded up to the nearest byte.
*/
extern size_t bits2bytes(size_t bits);

/**
 * Converts an exponent to the corresponding size in bytes.
 *
 * @param e The exponent.
 * @return The size in bytes corresponding to the exponent.
*/
extern size_t e2size(int e);

/**
 * Converts a size in bytes to the corresponding exponent.
 *
 * @param size The size in bytes.
 * @return The exponent corresponding to the size in bytes.
*/
extern int size2e(size_t size);

//Sets provided bit in provided pointer to 1
extern void bitset(void *p, int bit);
//Clears provided bit in provided pointer to 0
extern void bitclr(void *p, int bit);
//Inverts provided bit in provided pointer
extern void bitinv(void *p, int bit);
//Tests provided bit in provided pointer, returns 1 if set, 0 if not
extern int  bittst(void *p, int bit);

#endif
