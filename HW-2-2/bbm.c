#include "bbm.h"
#include "bm.h"
#include "utils.h"

// Computes the size of the bitmap needed to track the given memory region at the specified block size exponent.
static size_t mapsize(size_t size, int e) {
  size_t blocksize=e2size(e);
  size_t blocks=divup(size,blocksize);
  size_t buddies=divup(blocks,2);
  return buddies;
}

// Computes the bit address within the bitmap for the given memory block and block size exponent.
static size_t bitaddr(void *base, void *mem, int e) {
  size_t addr=baddrclr(base,mem,e)-base;
  size_t blocksize=e2size(e);
  return addr/blocksize/2;
}

// Creates a new buddy bitmap for the given memory region and block size exponent.
extern BBM bbmcreate(size_t size, int e) {
  return bmcreate(mapsize(size,e));
}
// Deletes the buddy bitmap and frees its memory.
extern void bbmdelete(BBM b) {
  bmdelete(b);
}
// Sets the bit corresponding to the given memory block in the buddy bitmap.
extern void bbmset(BBM b, void *base, void *mem, int e) {
  bmset(b,bitaddr(base,mem,e));
}
// Clears the bit corresponding to the given memory block in the buddy bitmap.
extern void bbmclr(BBM b, void *base, void *mem, int e) {
  bmclr(b,bitaddr(base,mem,e));
}
// Tests whether the bit corresponding to the given memory block is set in the buddy bitmap.
extern int bbmtst(BBM b, void *base, void *mem, int e) {
  return bmtst(b,bitaddr(base,mem,e));
}
// Prints the buddy bitmap for debugging purposes.  
extern void bbmprt(BBM b) { bmprt(b); }

// Computes the address of the memory block with the specified bit set in the buddy bitmap.
extern void *baddrset(void *base, void *mem, int e) {
  unsigned int mask=1<<e;
  return base+((mem-base)|mask);
}
// Computes the address of the memory block with the specified bit cleared in the buddy bitmap.
extern void *baddrclr(void *base, void *mem, int e) {
  unsigned int mask=~(1<<e);
  return base+((mem-base)&mask);
}

// Computes the address of the memory block with the specified bit inverted in the buddy bitmap.
extern void *baddrinv(void *base, void *mem, int e) {
  unsigned int mask=1<<e;
  return base+((mem-base)^mask);
}

// Tests whether the specified bit is set in the buddy bitmap.
extern int baddrtst(void *base, void *mem, int e) {
  unsigned int mask=1<<e;
  return (mem-base)&mask;
}
