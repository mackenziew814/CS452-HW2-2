#include <stdint.h>
#include <string.h>

#include "balloc.h"
#include "freelist.h"
#include "utils.h"

typedef struct {
  void    *raw;      
  size_t   rawsize;  
  void *base;
  int      l, u;
  FreeList f;
} *BP;

// Rounds p up to the next multiple of align, which must be a power of
// two. Used to find a 2^u-aligned address inside a plain mmap
// mapping, since mmap itself only guarantees page alignment.
static void *alignup(void *p, size_t align) {
  uintptr_t addr=(uintptr_t)p;
  uintptr_t mask=align-1;
  return (void *)((addr+mask)&~mask);
}

extern Balloc bcreate(unsigned int size, int l, int u) {
  // Free blocks store their "next" pointer in their own first bytes
  // (see freelist.c), so no block can ever be smaller than a
  // pointer. Silently raise l (and u along with it, if needed) rather
  // than let freelistalloc hand out a block too small to hold that
  // pointer -- this only ever changes anything for unrealistically
  // small l (on a 64-bit system, l<3 means blocks under 8 bytes).
  int minl=size2e(sizeof(void *));
  if (l<minl)
    l=minl;
  if (u<l)
    u=l;

  size_t blocksz=e2size(u);

  // Round the requested size up to a whole number of 2^u blocks, per
  // the assignment: a size that isn't a power of two, or isn't a
  // multiple of 2^u, still gets a reasonably-sized set of free lists
  // rather than failing outright. Always keep at least one top-level
  // block even if size was 0.
  size_t need=divup((size_t)size,blocksz)*blocksz;
  if (need<blocksz)
    need=blocksz;

  BP bp=mmalloc(sizeof *bp);
  if ((long)bp==-1)
    return 0;

  // Map 'need' bytes of usable region, plus one extra block's worth
  // of padding so there is always room to find a 2^u-aligned address
  // inside the mapping no matter where mmap actually placed it.
  size_t rawsize=need+blocksz;
  void *raw=mmalloc(rawsize);
  if ((long)raw==-1) {
    mmfree(bp,sizeof *bp);
    return 0;
  }
  void *base=alignup(raw,blocksz);

  FreeList f=freelistcreate(need,l,u);
  if (!f) {
    mmfree(raw,rawsize);
    mmfree(bp,sizeof *bp);
    return 0;
  }

  bp->raw=raw;
  bp->rawsize=rawsize;
  bp->base=base;
  bp->l=l;
  bp->u=u;
  bp->f=f;

  // Seed the free list: every 2^u-sized chunk of the region starts
  // out as one independent free block at the top level. These never
  // merge with each other (there is nothing above u to merge into),
  // but each is independently splittable down to level l as needed.
  int nblocks=need/blocksz;
  for (int i=0; i<nblocks; i++)
    freelistfree(f,base,(char *)base+i*blocksz,u,l);

  return bp;
}

extern void bdelete(Balloc ba) {
  BP bp = ba;
  freelistdelete(bp->f, bp->l, bp->u);
  mmfree(bp->raw, bp->rawsize);
  mmfree(bp, sizeof *bp);
}

extern void *balloc(Balloc ba, unsigned int size) {
  BP bp=ba;
  if (size==0){
    size=1;
  }

  int e=size2e(size);
  if (e < bp->l){
    e = bp->l;               // never smaller than the smallest allowed block
  }
  if (e > bp->u){
    return 0;              // request is too large
  }
  return freelistalloc(bp->f, bp->base, e, bp->l);
}

extern void bfree(Balloc ba, void *mem) {
  BP bp = ba;
  if (!mem) {
    return;
  }
  int e=freelistsize(bp->f, bp->base, mem, bp->l, bp->u);
  freelistfree(bp->f, bp->base, mem, e, bp->l);
}

extern unsigned int bsize(Balloc ba, void *mem) {
  BP bp = ba;
  int e = freelistsize(bp->f, bp->base, mem, bp->l, bp->u);
  return (unsigned int)e2size(e);
}

extern void bprint(Balloc ba) {
  BP bp = ba;
  printf("Balloc: base=%p l=%d u=%d\n",bp->base, bp->l ,bp->u);
  freelistprint(bp->f, bp->l, bp->u);
}