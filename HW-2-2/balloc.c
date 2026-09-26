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
  uintptr_t addr = (uintptr_t)p;
  uintptr_t mask = align - 1;
  return (void *)((addr+mask)&~mask);
}

extern Balloc bcreate(unsigned int size, int l, int u) {
  int minl = size2e(sizeof(void *));
  if (l < minl)
    l = minl;
  if (u < l)
    u = l;

  size_t blocksz=e2size(u);

  size_t need=divup((size_t)size,blocksz)*blocksz;
  if (need < blocksz)
    need = blocksz;

  BP bp=mmalloc(sizeof *bp);
  if ((long)bp == -1)
    return 0;

  size_t rawsize=need+blocksz;
  void *raw = mmalloc(rawsize);
  if ((long)raw == -1) {
    mmfree(bp, sizeof *bp);
    return 0;
  }
  void *base=alignup(raw,blocksz);

  FreeList f = freelistcreate(need, l, u);
  if (!f) {
    mmfree(raw, rawsize);
    mmfree(bp, sizeof *bp);
    return 0;
  }

  bp->raw = raw;
  bp->rawsize = rawsize;
  bp->base = base;
  bp->l = l;
  bp->u = u;
  bp->f = f;

  int nblocks = need / blocksz;
  for (int i = 0; i < nblocks; i++){
    freelistfree(f, base, (char *)base + i * blocksz, u, l);
  }
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
    e = bp->l;  // never smaller than the smallest allowed block
  }
  if (e > bp->u){
    return 0;  // request is too large
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
  size_t free = freelistfreebytes(bp->f, bp->l, bp->u);
 
  printf("Balloc: base=%p\n", bp->base);
  printf("  block sizes: 2^%d (%zu bytes) .. 2^%d (%zu bytes)\n",  bp->l, e2size(bp->l), bp->u, e2size(bp->u));
  printf("  region: %zu bytes total, %zu free, %zu allocated\n", bp->rawsize, free, bp->rawsize - free);
  printf("  free lists:\n");
  freelistprint(bp->f, bp->l, bp->u);
}