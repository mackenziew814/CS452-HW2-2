#ifndef FREELIST_H
#define FREELIST_H

#include <stdio.h>

typedef void *FreeList;

/**
 * Creates a free-list manager for a region of memory.
 *
 * @param size The total size of the managed memory region, in bytes.
 * @param l The minimum block exponent or lower bound for the free-list range.
 * @param u The maximum block exponent or upper bound for the free-list range.
 * @return A handle to the newly created free-list structure, or NULL on failure.
 */
extern FreeList freelistcreate(size_t size, int l, int u);

/**
 * Destroys a free-list manager and releases any resources it owns.
 *
 * @param f The free-list handle to destroy.
 * @param l The lower bound used when creating the free list.
 * @param u The upper bound used when creating the free list.
 */
extern void freelistdelete(FreeList f, int l, int u);

/**
 * Allocates a block of the requested size from the free list.
 *
 * @param f The free-list handle to use for allocation.
 * @param base The base address of the managed memory region.
 * @param e The requested block size exponent.
 * @param l The lower bound for the allocator's range.
 * @return A pointer to the allocated memory block, or NULL if no block is available.
 */
extern void *freelistalloc(FreeList f, void *base, int e, int l);

/**
 * Returns a previously allocated block to the free list.
 *
 * @param f The free-list handle to use for reclamation.
 * @param base The base address of the managed memory region.
 * @param mem The block being returned to the free list.
 * @param e The block size exponent for the returned region.
 * @param l The lower bound for the allocator's range.
 */
extern void freelistfree(FreeList f, void *base, void *mem, int e, int l);

/**
 * Computes the total number of free entries or allocated block count represented
 * by the free list for the given memory region.
 *
 * @param f The free-list handle to inspect.
 * @param base The base address of the managed memory region.
 * @param mem The memory block whose size or free-list entry is being queried.
 * @param l The lower bound for the free-list range.
 * @param u The upper bound for the free-list range.
 * @return The size or count associated with the queried free-list entry.
 */
extern int freelistsize(FreeList f, void *base, void *mem, int l, int u);

/**
 * Prints the current contents of the free list for debugging or inspection.
 *
 * @param f The free-list handle to display.
 * @param l The lower bound for the free-list range.
 * @param u The upper bound for the free-list range.
 */
extern void freelistprint(FreeList f, int l, int u);

#endif
