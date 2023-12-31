#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"

int sys_fork(void)
{
  return fork();
}

int sys_exit(void)
{
  exit();
  return 0; // not reached
}

int sys_wait(void)
{
  return wait();
}

int sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void)
{
  return myproc()->pid;
}

int sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

struct sleeplock sleep_priority_lock;

void print_p_queue(int *queue)
{
  int i = 0;
  cprintf("queue: ");
  while (queue[i] != EMPTY_PID)
  {
    cprintf("%d ", queue[i]);
    i++;
  }

  cprintf("\n");
}

int sys_acquire_priority_lock(void)
{

  if (sleep_priority_lock.is_initilized == 0)
  {
    init_p_lock(&sleep_priority_lock, "sleep_priority_lock");
  }

  cprintf("asking to acquire lock: %d\n", myproc()->pid);
  acquire_p_lock(&sleep_priority_lock, myproc()->pid);
  cprintf("acquired lock: %d\n", myproc()->pid);
  print_p_queue(sleep_priority_lock.pid_queue);
  return 0;
}

int sys_release_priority_lock(void)
{
  if (holding_p_lock(&sleep_priority_lock))
  {
    cprintf("released the lock: %d\n", myproc()->pid);
    release_p_lock(&sleep_priority_lock, myproc()->pid);
  }
  else
  {
    cprintf("process %d does not hold the lock and cannot release it\n", myproc()->pid);
  }

  return 0;
}