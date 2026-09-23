#ifndef BALLOC_H
#define BALLOC_H

typedef void *Balloc;

// Create a memory pool
extern Balloc bcreate(unsigned int size, int l, int u);
//Delete the memory pool
extern void   bdelete(Balloc pool);

// Allocate a memory block from the pool
extern void *balloc(Balloc pool, unsigned int size);
// Free a memory block
extern void  bfree(Balloc pool, void *mem);

// Get the size of a memory block
extern unsigned int bsize(Balloc pool, void *mem);

// Print the state of the memory pool
extern void bprint(Balloc pool);

#endif
