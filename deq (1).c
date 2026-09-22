#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum {Head,Tail,Ends} End;

typedef struct Node {
  struct Node *np[Ends];        // next/prev neighbors
  Data data;
} *Node;

typedef struct {
  Node ht[Ends];                // head/tail nodes
  int len;
} *Rep;

static Rep rep(Deq q) {
  if (!q) { return 0; } //returns 0
  return (Rep)q;
}

// Return the other end of the deque
static End other(End e) {
    return e==Head ? Tail : Head;
 }

//Inserts new node into queue towards the end specified
//Return 0 on invalid input
static void put(Rep r, End e, Data d) {
  End otherEnd = other(e);
  Node n = (Node)malloc(sizeof(*n));
  if (!n){
      ERROR("malloc() failed");
  }

  n->data = d;
  n->np[e] = 0; //New node is now the head or tail
  n->np[otherEnd] = r->ht[e]; //New node points to old node

  //Checks if end is head or tail
  if (r->ht[e])
    r->ht[e]->np[e] = n; // sets new node as new head or tail
  else
    r->ht[otherEnd] = n; // deque was empty
 
  r->ht[e] = n;
  r->len++;

}

//Returns without removing the data at the specified index
//Returns 0 if given improper input
static Data ith(Rep r, End e, int i)  {
  if(!r || i >= r->len || i < 0){
    return 0;
  }

  End otherEnd = other(e);
  Node n = r->ht[e];

  for(int j = 0; j < i; j++){
    n = n->np[otherEnd];
  }
  
 return n->data;
}

//Returns and removes data at end e
//Returns 0 if given improper input
static Data get(Rep r, End e) {
  if(!r || r->len <= 0){
    return 0;
  }

  End otherEnd = other(e);
  Node n = r->ht[e];
  Data d = n->data;

  r->ht[e] = n->np[otherEnd];
  if (r->ht[e]){
    r->ht[e]->np[e] = 0;   // new head or tail
  }
  else {
    r->ht[otherEnd] = 0; //Queue is now emtpy
  }

  free(n);
  r->len--;
  return d;
}

//Remove specified data from queue 
//Returns 0 if given improper input
static Data rem(Rep r, End e, Data d) {
  if(!r){
    return 0;
  }

  End otherEnd = other(e);
  Node n = r->ht[e];
  int i;
  //Walk through queue until data is found
  for (i = 0; i < r->len && n->data != d; i++){
    n=n->np[otherEnd];
  }

  //Return 0 if not found
  if (i == r->len){
    return 0;
  } 

  //Update pointers on provided end
  if (n->np[e]){
    n->np[e]->np[otherEnd] = n->np[otherEnd];
  }  
  else {
    r->ht[e] = n->np[otherEnd];
  } 

  //Update pointers on the opposite end
  if (n->np[otherEnd]){
    n->np[otherEnd]->np[e] = n->np[e];
  }  
  else{
    r->ht[otherEnd] = n->np[e];
  } 

  r->len--;
  free(n);
  return d;
}

extern Deq deq_new() {
  Rep r=(Rep)malloc(sizeof(*r));
  if (!r) ERROR("malloc() failed");
  r->ht[Head]=0;
  r->ht[Tail]=0;
  r->len=0;
  return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) {        put(rep(q),Head,d); }
extern Data deq_head_get(Deq q)         { return get(rep(q),Head);   }
extern Data deq_head_ith(Deq q, int i)  { return ith(rep(q),Head,i); }
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q),Head,d); }

extern void deq_tail_put(Deq q, Data d) {        put(rep(q),Tail,d); }
extern Data deq_tail_get(Deq q)         { return get(rep(q),Tail);   }
extern Data deq_tail_ith(Deq q, int i)  { return ith(rep(q),Tail,i); }
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q),Tail,d); }

extern void deq_map(Deq q, DeqMapF f) {
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail])
    f(n->data);
}

extern void deq_del(Deq q, DeqMapF f) {
  if (f) deq_map(q,f);
  Node curr=rep(q)->ht[Head];
  while (curr) {
    Node next=curr->np[Tail];
    free(curr);
    curr=next;
  }
  free(q);
}

extern Str deq_str(Deq q, DeqStrF f) {
  char *s=strdup("");
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail]) {
    char *d=f ? f(n->data) : n->data;
    char *t; asprintf(&t,"%s%s%s",s,(*s ? " " : ""),d);
    free(s); s=t;
    if (f) free(d);
  }
  return s;
}
