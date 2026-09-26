// Exercises deq.c through the malloc/free/realloc wrapper
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "deq.h"

int testsRun = 0;
int testsFailed = 0;

static void testCheck(const char *testName, int passed) {
    testsRun++;
    if (!passed) {
        testsFailed++;
        printf("Test failed: %s\n", testName);
    }   
}

// Boxes an int on the heap so it can be stored as Data in the deque.
static int *box(int v) {
  int *p=malloc(sizeof *p);
  *p=v;
  return p;
}

static char *intstr(Data d) {
  char *s;
  asprintf(&s,"%d",*(int *)d);
  return s;
}

static int sum;
static void addup(Data d) {
   sum += *(int *)d;
 }

static void freebox(Data d) {
   free(d);
 }

int main(void) {
  Deq q = deq_new();
  testCheck("deq_new returns a non-null queue", q != 0);
  testCheck("fresh queue has length 0", deq_len(q) == 0);

  // push 0-9 onto the tail, 10-19 onto the head 
  for (int i=0; i<10; i++) {
    deq_tail_put(q, box(i));
    deq_head_put(q, box(100 + i));
  }
  testCheck("length after 20 puts is 20",deq_len(q) == 20);

  // deq_head_ith should read the most-recently-head-pushed value first
  int *first=deq_head_ith(q, 0);
  testCheck("head_ith(0) is the last head_put value", *first == 109);

  // pop everything from both ends, verify order, verify no leftover nodes are double-freed
  int gothead=0;
  int gottail=0;

  while (deq_len(q) > 0) {
    if (deq_len(q) % 2 == 0) {
      int *d = deq_head_get(q);
      gothead++;
      free(d);
    } else {
      int *d = deq_tail_get(q);
      gottail++;
      free(d);
    }
  }
  testCheck("everything popped off (head+tail counts add up to 20)", gothead + gottail == 20);
  testCheck("queue is empty after popping everything", deq_len(q) == 0);

  // refill and test rem-by-pointer
  int *victim=box(42);
  deq_tail_put(q, box(1));
  deq_tail_put(q, victim);
  deq_tail_put(q, box(2));
  testCheck("length is 3 before rem", deq_len(q) == 3);
  Data removed=deq_tail_rem(q, victim);
  testCheck("rem returns the same pointer that was removed", removed==victim);
  testCheck("length is 2 after rem", deq_len(q) == 2);
  free(victim);

  // deq_map: sum the remaining boxed ints (should be 1 and 2)
  sum=0;
  deq_map(q, addup);
  testCheck("map correctly sums remaining elements (1+2=3)",sum == 3);

  // deq_str
  char *s = deq_str(q, intstr);
  testCheck("deq_str produced a non-null string", s != 0);
  testCheck("deq_str output matches expected \"1 2\"", strcmp(s, "1 2") == 0);
  free(s);

  // deq_del frees every box
  deq_del(q,freebox);

  // A second, independent queue afterward proves the allocator is still good
  Deq q2=deq_new();
  for (int i = 0; i < 50; i++)
    deq_tail_put(q2, box(i));
  testCheck("second queue after deq_del works and reaches length 50", deq_len(q2) == 50);
  sum=0;
  deq_map(q2, addup);
  testCheck("second queue's sum is correct (0..49 = 1225)", sum == 1225);
  deq_del(q2,freebox);

  printf("%d/%d tests passed\n", testsRun-testsFailed, testsRun);
  return testsFailed==0 ? 0 : 1;
}