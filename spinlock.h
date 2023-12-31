// Mutual exclusion lock.
struct spinlock
{
  uint locked; // Is the lock held?

  // For debugging:
  char *name;      // Name of lock.
  struct cpu *cpu; // The cpu holding the lock.
  uint pcs[10];    // The call stack (an array of program counters)
                   // that locked the lock.
  int *pid_queue;
  int is_initilized;
};
void initlock(struct spinlock *lk, char *name);
void get_priority_lock(struct spinlock *lk, int pid);
void free_priority_lock(struct spinlock *lk, int pid);