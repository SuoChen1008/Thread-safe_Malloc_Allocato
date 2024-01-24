#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "my_malloc.h"

void *ts_malloc_lock(size_t size){
    pthread_mutex_lock(&lock);
    void * res = bf_malloc_withlock(size);
    pthread_mutex_unlock(&lock);
    return res;
}

void ts_free_lock(void *ptr){
    pthread_mutex_lock(&lock);
    bf_free(ptr, free_head, free_tail);
    pthread_mutex_unlock(&lock);
}

void *ts_malloc_nolock(size_t size){
  void * res = bf_malloc_nolock(size);
  return res;
}

void ts_free_nolock(void *ptr){
  bf_free(ptr, nolock_free_head, nolock_free_tail);
}

void * bf_malloc_withlock(size_t size){
    if (free_head == NULL){
      Blk * nb = sbrk(BLK_SIZE + size);
      nb->next = NULL;
      nb->prev = NULL;
      nb->size = size;
      return (void *)((char *) nb + BLK_SIZE);
    }
    Blk *current = free_head;
    Blk *ans = NULL;
    int min_diff=INT_MAX;
    int diff=0;
    while (current!= NULL) {
      if (current->size< size){
        current=current->next;
      }
      else if (current->size == size){
        removeFreeBlk(current,free_head, free_tail);
        return (void *)((char *)current + BLK_SIZE);
      }
      else{
        diff = current->size - size;
        if (diff<min_diff){
          min_diff=diff;
          ans=current;
        }
        current = current->next;
      }
    }
    if (min_diff == INT_MAX){
      Blk * nb = sbrk(BLK_SIZE + size);
      nb->next = NULL;
      nb->prev = NULL;
      nb->size = size;
      return (void *)((char *) nb + BLK_SIZE);
    }
    split(size, ans,free_head,free_tail);
    return (void *)((char *) ans + BLK_SIZE);
    
}


void removeFreeBlk(Blk * BlkToRemove,Blk * head, Blk * tail) {
    if (BlkToRemove==NULL){
      return;
    }
    if (BlkToRemove != NULL && BlkToRemove->next == NULL && BlkToRemove->prev == NULL) {
      head = NULL;
      tail = NULL;
    }

    // if the block removed is the first block
    else if (head == BlkToRemove) {
      head = BlkToRemove->next;
      BlkToRemove->next=NULL;
      BlkToRemove->prev = NULL;
    }
    // if the block removed is the last block
    else if (tail == BlkToRemove){
      tail = BlkToRemove->prev;
      BlkToRemove->prev = NULL;
      tail->next=NULL;
    }
    // if the block removed is in the middle
    else {
      BlkToRemove->prev->next = BlkToRemove->next;
      BlkToRemove->next->prev = BlkToRemove->prev;
      BlkToRemove->next=NULL;
      BlkToRemove->prev = NULL;
    }
}

void split(size_t size, Blk * BlkToSplit,Blk * head, Blk * tail) {
    if ( BlkToSplit != NULL && BlkToSplit->size > size + BLK_SIZE){
        Blk * secondBlk = (Blk *)((char *)BlkToSplit + size + BLK_SIZE);
        secondBlk->prev = NULL;
        secondBlk->next = NULL;
        secondBlk->size = BlkToSplit->size - size - BLK_SIZE;
        BlkToSplit->size = size;
        removeFreeBlk(BlkToSplit, head, tail);
        addFreeBlk(secondBlk, head, tail);
    }
    else{
        removeFreeBlk(BlkToSplit, head, tail);
    }
}



void bf_free(void * ptr,Blk * head, Blk * tail) {
    Blk * BlktoFree = (Blk *)((char *)ptr - BLK_SIZE);
    addFreeBlk(BlktoFree,head,tail);
    merge(BlktoFree,tail);
}

void addFreeBlk(Blk * BlkToAdd,Blk * head, Blk * tail){
  if (head == NULL) {
    head = BlkToAdd;
    tail = BlkToAdd;
  }
  else if (BlkToAdd < head) {
    BlkToAdd->next=head;
    head->prev = BlkToAdd;
    head = BlkToAdd;
  }
  else {
    Blk *current = head;
    while (current != NULL) {
      if (current < BlkToAdd) {
        if (current->next > BlkToAdd) {
          BlkToAdd->prev = current;
          BlkToAdd->next = current->next;
          BlkToAdd->prev->next->prev = BlkToAdd;
          BlkToAdd->prev->next = BlkToAdd;
          break;
        }
        else if (current->next== NULL) {
          tail->next = BlkToAdd;
          BlkToAdd->prev = tail;
          tail = BlkToAdd;
          break;
        }
        else{
        current = current->next;
        }
      }
    }
  }

}

void merge(Blk * block, Blk * tail) {
      if (block->prev != NULL && block == (Blk *)((char *)block->prev + block->prev->size + BLK_SIZE)) {
        block->prev->size = block->prev->size + block->size+ BLK_SIZE;
        block->prev->next = block->next;
        if (block!= tail) {
          block->next->prev = block->prev;
        }
        else{
          tail = block->prev;
        }
        block = block->prev;
    }
      if (block->next != NULL && block->next == (Blk *)((char *)block + block ->size + BLK_SIZE)) {
        block->size = block->size + block->next->size + BLK_SIZE;
        block->next = block->next->next;
       if (block->next== NULL) {
          tail = block;
        }
        else {
          block->next->prev = block;
        }
      }
}

void * bf_malloc_nolock(size_t size){
    if (nolock_free_head == NULL){
        pthread_mutex_lock(&lock);
        Blk * nb = sbrk(BLK_SIZE + size);
        pthread_mutex_unlock(&lock);
        nb->next = NULL;
        nb->prev = NULL;
        nb->size = size;
        return (void *)((char *) nb + BLK_SIZE);
    }
    Blk *current = nolock_free_head;
    Blk *ans = NULL;
    int min_diff=INT_MAX;
    int diff=0;
    while (current!= NULL) {
      if (current->size< size){
        current=current->next;
      }
      else if (current->size == size){
        removeFreeBlk(current,nolock_free_head,nolock_free_tail );
        return (void *)((char *)current + BLK_SIZE);
      }
      else{
        diff = current->size - size;
        if (diff<min_diff){
          min_diff=diff;
          ans=current;
        }
        current = current->next;
      }
    }
    if (min_diff == INT_MAX){
        pthread_mutex_lock(&lock);
        Blk * nb = sbrk(BLK_SIZE + size);
        pthread_mutex_unlock(&lock);
        nb->next = NULL;
        nb->prev = NULL;
        nb->size = size;
        return (void *)((char *) nb + BLK_SIZE);
    }
    split(size, ans,nolock_free_head, nolock_free_tail);
    return (void *)((char *) ans + BLK_SIZE);
    
}
