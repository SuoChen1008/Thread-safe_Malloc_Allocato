
#include <unistd.h>
#include <pthread.h>


// Block strcut
typedef struct MyBlk {
  size_t size;             
  struct MyBlk * prev;  
  struct MyBlk * next;  
}Blk;

#define BLK_SIZE sizeof(Blk)

Blk *free_head = NULL;
Blk *free_tail = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
__thread Blk * nolock_free_head = NULL;
__thread Blk * nolock_free_tail = NULL;

// Best fit malloc/free
void * bf_malloc_withlock(size_t size);
void bf_free(void * ptr,Blk * head, Blk * tail);
void * bf_malloc_nolock(size_t size);


void addFreeBlk(Blk * BlkToAdd,Blk * head, Blk * tail);
void removeFreeBlk(Blk * BlkToRemove,Blk * head, Blk * tail);
void split(size_t size, Blk * BlkToSplit,Blk * head, Blk * tail);
void merge(Blk * block,Blk * tail);
//Thread Safe malloc/free: locking version void *ts_malloc_lock(size_t size);

void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

//Thread Safe malloc/free: non-locking version void *ts_malloc_nolock(size_t size);
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);