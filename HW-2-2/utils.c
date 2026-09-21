#include "utils.h"

static const int bitsperbyte=8;

extern void *mmalloc(size_t size){
  size = divup(size, pagesize()) * pagesize();
  return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

extern void mmfree(void *p, size_t size){
  size = divup(size, pagesize()) * pagesize();
  if (munmap(p, size) == -1) {
    perror("munmap");
  }
}

extern size_t divup(size_t n, size_t d){
  return (n + d - 1) / d;
}

extern size_t bits2bytes(size_t bits){
  return divup(bits, bitsperbyte);
}

extern size_t e2size(int e){
  return (size_t)1 << e;
}

extern int size2e(size_t size){
  int e = 0;
  size_t block = 1;
  while (block < size) {
    block <<= 1;
    e++;
  }
  return e;
}

extern void bitset(void *p, int bit) {
  unsigned char *c = p;
  *c|=(1u<<bit);
}

extern void bitclr(void *p, int bit) {
  unsigned char *c=p;
  *c &= ~(1u<<bit);
}

extern void bitinv(void *p, int bit) {
  unsigned char *c=p;
  *c ^= (1u<<bit);
}

extern int bittst(void *p, int bit) {
  unsigned char *c=p;
  return (*c & (1u<<bit)) != 0;
}