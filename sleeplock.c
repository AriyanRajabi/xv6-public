// Sleeping locks

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"

void initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void acquiresleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  while (lk->locked)
  {
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  release(&lk->lk);
}

void releasesleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);
}

int holdingsleep(struct sleeplock *lk)
{
  int r;

  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}

// lab 4
void add_to_p_queue(int *queue, int pid)
{
  int i;
  for (i = 0; i < MAX_QUEUE_SIZE; i++)
  {
    if (queue[i] == EMPTY_PID)
    {
      queue[i] = pid;
      break;
    }
  }
}

void remove_from_p_queue(int *queue, int pid)
{
  int i;
  for (i = 0; i < MAX_QUEUE_SIZE; i++)
  {
    if (queue[i] == pid)
    {
      // Shift all elements to the left
      for (int j = i; j < MAX_QUEUE_SIZE - 1; j++)
      {
        queue[j] = queue[j + 1];
      }
      // Set the last element to EMPTY_PID
      queue[MAX_QUEUE_SIZE - 1] = EMPTY_PID;
      break;
    }
  }
}

void sort_p_queue(int *queue) // using bubble sort, not very efficient, but useful
{
  int i, j, temp;
  for (i = 0; i < MAX_QUEUE_SIZE; i++)
  {
    for (j = i + 1; j < MAX_QUEUE_SIZE; j++)
    {
      if (queue[i] > queue[j])
      {
        temp = queue[i];
        queue[i] = queue[j];
        queue[j] = temp;
      }
    }
  }
}

void init_p_lock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
  lk->pid_queue = (int *)kalloc(); // TODO: if returns 0 -> failed
  for (int i = 0; i < MAX_QUEUE_SIZE; i++)
  {
    lk->pid_queue[i] = EMPTY_PID;
  }
  lk->is_initilized = 1;
}

void acquire_p_lock(struct sleeplock *lk, int pid)
{
  acquire(&lk->lk);
  add_to_p_queue(lk->pid_queue, pid);
  sort_p_queue(lk->pid_queue);
  while (lk->locked)
  {
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  release(&lk->lk);
}

void release_p_lock(struct sleeplock *lk, int pid)
{
  acquire(&lk->lk);
  remove_from_p_queue(lk->pid_queue, pid);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);
}

int holding_p_lock(struct sleeplock *lk)
{
  int r;

  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}
