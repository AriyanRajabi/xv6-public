// Mutual exclusion spin locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#define MAX_QUEUE_SIZE 1024  // kalloc allocates 4096 bytes (1 page) which is 1024 ints
#define EMPTY_PID 2147483647 // max int in c, because we want empty ones to be sorted to the end of the array (queue)

void initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpu = 0;
  lk->pid_queue = (int *)kalloc(); // TODO: if returns 0 -> failed
  for (int i = 0; i < MAX_QUEUE_SIZE; i++)
  {
    lk->pid_queue[i] = EMPTY_PID;
  }
  lk->is_initilized = 1;
}

void free_lock(struct spinlock *lk)
{
  kfree((char *)lk->pid_queue);
}

void add_to_queue(int *queue, int pid)
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

void remove_from_queue(int *queue, int pid)
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

void sort_queue(int *queue) // using bubble sort, not very efficient, but useful
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

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.

void acquire(struct spinlock *lk)
{
  pushcli(); // disable interrupts to avoid deadlock.
  if (holding(lk))
    panic("acquire");

  // The xchg is atomic.
  // stay in the loop until the current process is the highest priority
  while (xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();

  // Record info about lock acquisition for debugging.
  lk->cpu = mycpu();
  getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
void release(struct spinlock *lk)
{
  if (!holding(lk))
    panic("release");

  lk->pcs[0] = 0;
  lk->cpu = 0;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both not to.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code can't use a C assignment, since it might
  // not be atomic. A real OS would use C atomics here.
  asm volatile("movl $0, %0" : "+m"(lk->locked) :);

  popcli();
}

void get_priority_lock(struct spinlock *lk, int pid) // NOTE: we've implemented priority lock as a different function
{
  // pushcli(); // disable interrupts to avoid deadlock.
  if (holding(lk))
    panic("get_priority_lock");

  // Add current process to priority queue and sort
  add_to_queue(lk->pid_queue, pid);
  sort_queue(lk->pid_queue);
  cprintf("lk->pid_queue[0] = %d, pid=%d\n", lk->pid_queue[0], pid);

  // The xchg is atomic.
  // stay in the loop until the current process is the highest priority
  while (xchg(&lk->locked, 1) != 0 || lk->pid_queue[0] != pid)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();

  // Record info about lock acquisition for debugging.
  // lk->cpu = mycpu();
  getcallerpcs(&lk, lk->pcs);
}

void free_priority_lock(struct spinlock *lk, int pid)
{
  /*if (!holding(lk))
    panic("release");*/

  lk->pcs[0] = 0;
  lk->cpu = 0;

  // Remove current process from priority queue
  remove_from_queue(lk->pid_queue, pid);

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both not to.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code can't use a C assignment, since it might
  // not be atomic. A real OS would use C atomics here.
  asm volatile("movl $0, %0" : "+m"(lk->locked) :);

  // popcli();
}

// Record the current call stack in pcs[] by following the %ebp chain.
void getcallerpcs(void *v, uint pcs[])
{
  uint *ebp;
  int i;

  ebp = (uint *)v - 2;
  for (i = 0; i < 10; i++)
  {
    if (ebp == 0 || ebp < (uint *)KERNBASE || ebp == (uint *)0xffffffff)
      break;
    pcs[i] = ebp[1];      // saved %eip
    ebp = (uint *)ebp[0]; // saved %ebp
  }
  for (; i < 10; i++)
    pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int holding(struct spinlock *lock)
{
  int r;
  pushcli();
  r = lock->locked && lock->cpu == mycpu();
  popcli();
  return r;
}

// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void pushcli(void)
{
  // cprintf("push cli called");
  int eflags;

  eflags = readeflags();
  cli();
  if (mycpu()->ncli == 0)
    mycpu()->intena = eflags & FL_IF;
  mycpu()->ncli += 1;
}

void popcli(void)
{
  // cprintf("pop cli called");
  if (readeflags() & FL_IF)
    panic("popcli - interruptible");
  if (--mycpu()->ncli < 0)
    panic("popcli");
  if (mycpu()->ncli == 0 && mycpu()->intena)
    sti();
}
